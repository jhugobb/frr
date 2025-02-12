#include "SSAO.h"
#include "glwidget.h"


SSAO::~SSAO() {
  cleanUp();
}

void SSAO::onSceneClear() {
  cleanUp();
}


void SSAO::updateFPS() {
  current = previous;
  previous = 0;
  glwidget()->update();
}

void SSAO::cleanUp() {
  GLWidget &g = *glwidget();
  g.makeCurrent();
  g.glDeleteBuffers(coordBuffers.size(),  &coordBuffers[0]);
  g.glDeleteBuffers(normalBuffers.size(), &normalBuffers[0]);
  g.glDeleteBuffers(stBuffers.size(),  &stBuffers[0]);
  g.glDeleteBuffers(colorBuffers.size(),  &colorBuffers[0]);
  g.glDeleteVertexArrays(VAOs.size(), &VAOs[0]);
  g.glDeleteBuffers(textures_diffuse.size(), &textures_diffuse[0]);
  g.glDeleteBuffers(1, &gBuffer);
  g.glDeleteVertexArrays(1, &quadVAO);
  coordBuffers.clear();
  normalBuffers.clear();
  stBuffers.clear();
  colorBuffers.clear();
  VAOs.clear();
  numIndices.clear();
  textures_diffuse.clear();
}


void SSAO::onPluginLoad()
{
	GLWidget &g = *glwidget();
  g.clearScene();
	g.makeCurrent();
	// Resize to power-of-two viewport

  setUpShaders();
	
  g.glEnable(GL_DEPTH_TEST);
			
	for(unsigned int i=0; i<scene()->objects().size(); i++)
    addVBO(i);

  cout << scene()->objects().size() << endl;
  float linear = 0.7;
  float constant = 1;
  float quadratic = 1.8;
  radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f)))) / (2.0f * quadratic);

  kernelSize = 64;
  radius = 0.5;
  bias = 0.025;
  setFrameBuffer();
  genQuad();
  genKernels();

  // Initialises FPS counter
  current=previous=0;
  QTimer *timer=new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateFPS()));
  timer->start(1000);

  useAO = false;
  useBlur = false;
  useNormal = true;
  g.resize(1280, 720);
}

void SSAO::setUpShaders() {
	ssaovs = new QOpenGLShader(QOpenGLShader::Vertex, this);
	ssaovs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/ssao.vert");
	ssaofs = new QOpenGLShader(QOpenGLShader::Fragment, this);
	ssaofs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/ssao.frag");
  ssaoProgram = new QOpenGLShaderProgram(this);
	ssaoProgram->addShader(ssaovs);
	ssaoProgram->addShader(ssaofs);
	ssaoProgram->link();

	nossaofs = new QOpenGLShader(QOpenGLShader::Fragment, this);
	nossaofs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/nossao.frag");
  nossaoProgram = new QOpenGLShaderProgram(this);
  nossaoProgram->addShader(ssaovs);
  nossaoProgram->addShader(nossaofs);
  nossaoProgram->link();

  sepssaoXfs = new QOpenGLShader(QOpenGLShader::Fragment, this);
	sepssaoXfs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/sepssaoX.frag");
  sepssaoXProgram = new QOpenGLShaderProgram(this);
  sepssaoXProgram->addShader(ssaovs);
  sepssaoXProgram->addShader(sepssaoXfs);
  sepssaoXProgram->link();

  sepssaoYfs = new QOpenGLShader(QOpenGLShader::Fragment, this);
	sepssaoYfs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/sepssaoY.frag");
  sepssaoYProgram = new QOpenGLShaderProgram(this);
  sepssaoYProgram->addShader(ssaovs);
  sepssaoYProgram->addShader(sepssaoYfs);
  sepssaoYProgram->link();

  blurfs = new QOpenGLShader(QOpenGLShader::Fragment, this);
  blurfs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/blurssao.frag");
  blurProgram = new QOpenGLShaderProgram(this);
  blurProgram->addShader(ssaovs);
  blurProgram->addShader(blurfs);
  blurProgram->link();

	gvs = new QOpenGLShader(QOpenGLShader::Vertex, this);
	gvs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/gbuffer.vert");
	gfs = new QOpenGLShader(QOpenGLShader::Fragment, this);
	gfs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/gbuffer.frag");
	gprogram = new QOpenGLShaderProgram(this);
  gprogram->addShader(gvs);
	gprogram->addShader(gfs);
	gprogram->link();

  lightvs = new QOpenGLShader(QOpenGLShader::Vertex, this);
	lightvs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/light.vert");
	lightfs = new QOpenGLShader(QOpenGLShader::Fragment, this);
	lightfs->compileSourceFile(glwidget()->getPluginPath()+"/../SSAO/shaders/light.frag");
  lightProgram = new QOpenGLShaderProgram(this);
  lightProgram->addShader(lightvs);
	lightProgram->addShader(lightfs);
  lightProgram->link();

	if (!ssaoProgram->isLinked()) cout << "Shader link error" << endl; 
  if (!lightProgram->isLinked()) cout << "Shader link error" << endl; 
	if (!gprogram->isLinked()) cout << "Shader link error" << endl; 
}

void SSAO::preFrame()
{
	
}

void SSAO::postFrame()
{
  painter.begin(glwidget());
  painter.setPen(Qt::white);
  painter.drawText(10,15, QString("%0 fps").arg(current));
  painter.drawText(10,35, QString("SSAO: %0").arg(useAO));
  painter.drawText(10,55, QString("Separable: %0").arg(!useNormal));
  painter.drawText(10,75, QString("Blur: %0").arg(useBlur));
  painter.drawText(10,95, QString("Radius: %0").arg(radius));
  painter.drawText(10,115, QString("Sample Size: %0").arg(kernelSize));
  painter.end();

  ++previous;
}

void SSAO::onObjectAdd()
{
	addVBO( scene()->objects().size() - 1 );
}

bool SSAO::drawObjectFB(unsigned int i)
{
  GLWidget &g = *glwidget();

	g.glBindVertexArray(VAOs[i]);
 
	g.glDrawArrays(GL_TRIANGLES, 0, numIndices[i]);
	
  g.glBindVertexArray(0);
	return true; // return true only if implemented
}

void SSAO::drawQuad()
{
  GLWidget &g = *glwidget();

  g.glBindVertexArray(quadVAO);
  g.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  g.glBindVertexArray(0);
}

void SSAO::regularAO() {
  GLWidget &g = *glwidget();
  ssaoBuf->bind();
  g.glClear(GL_COLOR_BUFFER_BIT);
    ssaoProgram->bind();
    ssaoProgram->setUniformValue("gPosition", 0);
    ssaoProgram->setUniformValue("gNormal", 1);
    ssaoProgram->setUniformValue("texNoise", 2);
    ssaoProgram->setUniformValue("kernelSize", kernelSize);
    ssaoProgram->setUniformValue("radius", radius);
    ssaoProgram->setUniformValue("bias", bias);

    ssaoProgram->setUniformValueArray("samples", ssaoKernel.data(), ssaoKernel.size());
    ssaoProgram->setUniformValue("projection", camera()->projectionMatrix());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuf->textures()[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuf->textures()[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    drawQuad();

    g.glActiveTexture(GL_TEXTURE0);
    g.glBindTexture(GL_TEXTURE_2D, 0);
    g.glActiveTexture(GL_TEXTURE1);
    g.glBindTexture(GL_TEXTURE_2D, 0);
    g.glActiveTexture(GL_TEXTURE2);
    g.glBindTexture(GL_TEXTURE_2D, 0);
    ssaoProgram->release();

  ssaoBuf->release();

  if (useBlur) {
    blurBuf->bind();
    g.glClear(GL_COLOR_BUFFER_BIT);
    blurProgram->bind();
    blurProgram->setUniformValue("ssaoInput", 0);

    g.glActiveTexture(GL_TEXTURE0);
    g.glBindTexture(GL_TEXTURE_2D, ssaoBuf->textures()[0]);

    drawQuad();

    g.glActiveTexture(GL_TEXTURE0);
    g.glBindTexture(GL_TEXTURE_2D, 0);

    blurProgram->release();
    blurBuf->release();
  }
}

void SSAO::separableAOX() {
  GLWidget &g = *glwidget();
  sepBufX->bind();
  g.glClear(GL_COLOR_BUFFER_BIT);
  sepssaoXProgram->bind();
  sepssaoXProgram->setUniformValue("gPosition", 0);
  sepssaoXProgram->setUniformValue("gNormal", 1);
  sepssaoXProgram->setUniformValue("texNoise", 2);
  sepssaoXProgram->setUniformValue("kernelSize", kernelSize);
  sepssaoXProgram->setUniformValue("radius", radius);
  sepssaoXProgram->setUniformValue("bias", bias);

  sepssaoXProgram->setUniformValueArray("samples", ssaoKernel.data(), ssaoKernel.size());
  sepssaoXProgram->setUniformValue("projection", camera()->projectionMatrix());

  g.glActiveTexture(GL_TEXTURE0);
  g.glBindTexture(GL_TEXTURE_2D, gBuf->textures()[0]);
  g.glActiveTexture(GL_TEXTURE1);
  g.glBindTexture(GL_TEXTURE_2D, gBuf->textures()[1]);
  g.glActiveTexture(GL_TEXTURE2);
  g.glBindTexture(GL_TEXTURE_2D, noiseTexture);

  drawQuad();

  g.glActiveTexture(GL_TEXTURE0);
  g.glBindTexture(GL_TEXTURE_2D, 0);
  g.glActiveTexture(GL_TEXTURE1);
  g.glBindTexture(GL_TEXTURE_2D, 0);
  g.glActiveTexture(GL_TEXTURE2);
  g.glBindTexture(GL_TEXTURE_2D, 0);

  sepssaoXProgram->release();
  sepBufX->release();

}

void SSAO::separableAOY() {
  GLWidget &g = *glwidget();
  sepBufY->bind();
  g.glClear(GL_COLOR_BUFFER_BIT);
    sepssaoYProgram->bind();
    sepssaoYProgram->setUniformValue("gPosition", 0);
    sepssaoYProgram->setUniformValue("gNormal", 1);
    sepssaoYProgram->setUniformValue("texNoise", 2);
    sepssaoYProgram->setUniformValue("xOcc", 3);
    sepssaoYProgram->setUniformValue("kernelSize", kernelSize);
    sepssaoYProgram->setUniformValue("radius", radius);
    sepssaoYProgram->setUniformValue("bias", bias);

    sepssaoYProgram->setUniformValueArray("samples", ssaoKernel.data(), ssaoKernel.size());
    sepssaoYProgram->setUniformValue("projection", camera()->projectionMatrix());

    g.glActiveTexture(GL_TEXTURE0);
    g.glBindTexture(GL_TEXTURE_2D, gBuf->textures()[0]);
    g.glActiveTexture(GL_TEXTURE1);
    g.glBindTexture(GL_TEXTURE_2D, gBuf->textures()[1]);
    g.glActiveTexture(GL_TEXTURE2);
    g.glBindTexture(GL_TEXTURE_2D, noiseTexture);
    g.glActiveTexture(GL_TEXTURE3);
    g.glBindTexture(GL_TEXTURE_2D, sepBufX->textures()[0]);

    drawQuad();

    g.glActiveTexture(GL_TEXTURE0);
    g.glBindTexture(GL_TEXTURE_2D, 0);
    g.glActiveTexture(GL_TEXTURE1);
    g.glBindTexture(GL_TEXTURE_2D, 0);
    g.glActiveTexture(GL_TEXTURE2);
    g.glBindTexture(GL_TEXTURE_2D, 0);
    g.glActiveTexture(GL_TEXTURE3);
    g.glBindTexture(GL_TEXTURE_2D, 0);

    sepssaoYProgram->release();
  sepBufY->release();

  if (useBlur) {
    blurBuf->bind();
    g.glClear(GL_COLOR_BUFFER_BIT);
    blurProgram->bind();
    blurProgram->setUniformValue("ssaoInput", 0);

    g.glActiveTexture(GL_TEXTURE0);
    g.glBindTexture(GL_TEXTURE_2D, sepBufY->textures()[0]);

    drawQuad();

    g.glActiveTexture(GL_TEXTURE0);
    g.glBindTexture(GL_TEXTURE_2D, 0);

    blurProgram->release();
    blurBuf->release();
  }
}

bool SSAO::paintGL()
{
	GLWidget &g = *glwidget();
	g.makeCurrent();
	g.glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// G buffer pass
	QMatrix4x4 MVP = camera()->projectionMatrix() * camera()->viewMatrix();
  gBuf->bind();
	gprogram->bind();
	gprogram->setUniformValue("view", camera()->viewMatrix());
  gprogram->setUniformValue("projection", camera()->projectionMatrix());
  g.glClearColor(0, 0, 0, 0);
  g.glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  for (unsigned int i=0; i < scene()->objects().size(); i++)
	  drawObjectFB(i);

  gBuf->release();
  gprogram->release();
  
  if (useAO) {
    if (useNormal) regularAO();
    else {
      separableAOX();
      separableAOY();
    }
  } else {
    ssaoBuf->bind();
    g.glClear(GL_COLOR_BUFFER_BIT);
    nossaoProgram->bind();
    drawQuad();
    nossaoProgram->release();
    ssaoBuf->release();
  }

  g.glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	lightProgram->bind();
	lightProgram->setUniformValue("viewPos", camera()->getObs());
  lightProgram->setUniformValue("gPosition", 0);
  lightProgram->setUniformValue("gNormal", 1);
  lightProgram->setUniformValue("gAlbedoSpec", 2);
  lightProgram->setUniformValue("ssao", 3);
  lightProgram->setUniformValue("radius", radius);
  lightProgram->setUniformValue("lightPosition", QVector3D(0, 1, 1));
  lightProgram->setUniformValue("lightColor", QVector3D(1,0,0));
  lightProgram->setUniformValue("MVP", MVP);

  g.glActiveTexture(GL_TEXTURE0);
  g.glBindTexture(GL_TEXTURE_2D, gBuf->textures()[0]);
  g.glActiveTexture(GL_TEXTURE1);
  g.glBindTexture(GL_TEXTURE_2D, gBuf->textures()[1]);
  g.glActiveTexture(GL_TEXTURE2);
  g.glBindTexture(GL_TEXTURE_2D, gBuf->textures()[2]);
  g.glActiveTexture(GL_TEXTURE3);
  if (!useAO) g.glBindTexture(GL_TEXTURE_2D, ssaoBuf->textures()[0]);
  else if (useBlur) g.glBindTexture(GL_TEXTURE_2D, blurBuf->textures()[0]);
  else if (useNormal) g.glBindTexture(GL_TEXTURE_2D, ssaoBuf->textures()[0]);
       else g.glBindTexture(GL_TEXTURE_2D, sepBufY->textures()[0]);
  drawQuad();

  g.glActiveTexture(GL_TEXTURE0);
  g.glBindTexture(GL_TEXTURE_2D, 0);
  g.glActiveTexture(GL_TEXTURE1);
  g.glBindTexture(GL_TEXTURE_2D, 0);
  g.glActiveTexture(GL_TEXTURE2);
  g.glBindTexture(GL_TEXTURE_2D, 0);
  g.glActiveTexture(GL_TEXTURE3);
  g.glBindTexture(GL_TEXTURE_2D, 0);
  lightProgram->release();
	
	return true;
}

void SSAO::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_C) {
    useAO = !useAO;
  } else if (e->key() == Qt::Key_B) {
    useBlur = !useBlur;
  } else if (e->key() == Qt::Key_K) {
    changeKernelSize();
  } else if (e->key() == Qt::Key_R) {
    changeRadius();
  } else if (e->key() == Qt::Key_P) {
    changeBias();
  } else if (e->key() == Qt::Key_Y) {
    useNormal = !useNormal;
  }
}

void SSAO::mouseMoveEvent(QMouseEvent *)
{
	
}

void SSAO::addVBO(unsigned int i) {
	const Object& obj = scene()->objects()[i];
  unsigned int numvertices = obj.faces().size()*3;  // it's all triangles...
  vector<float> vertices; // (x,y,z)    Final size: 9*number of triangles
  vector<float> normals;  // (nx,ny,nz) Final size: 9*number of triangles
  vector<float> colors;   // (r, g, b)  Final size: 9*number of triangles
  vector<float> texCoords;// (s, t)     Final size: 6*number of triangles
  auto verts = obj.vertices();
  auto Ns = obj.vertNormals();
  auto texcords = obj.vertTexCoords();

  for (auto&& f: obj.faces()) {
    Point P = verts[f.vertexIndex(0)].coord();
    vertices.push_back(P.x()); vertices.push_back(P.y()); vertices.push_back(P.z());
    Vector V=Ns[f.normalIndex(0)];
    normals.push_back(V.x()); normals.push_back(V.y()); normals.push_back(V.z());
    colors.push_back(fabs(V.x())); colors.push_back(fabs(V.y())); colors.push_back(fabs(V.z()));
    auto TC=texcords[f.texcoordsIndex(0)];
    texCoords.push_back(TC.first);  texCoords.push_back(TC.second);

    P = verts[f.vertexIndex(1)].coord();
    vertices.push_back(P.x()); vertices.push_back(P.y()); vertices.push_back(P.z());
    V=Ns[f.normalIndex(1)];
    normals.push_back(V.x()); normals.push_back(V.y()); normals.push_back(V.z());
    colors.push_back(fabs(V.x())); colors.push_back(fabs(V.y())); colors.push_back(fabs(V.z()));
    TC=texcords[f.texcoordsIndex(1)];
    texCoords.push_back(TC.first);  texCoords.push_back(TC.second);

    P = verts[f.vertexIndex(2)].coord();
    vertices.push_back(P.x()); vertices.push_back(P.y()); vertices.push_back(P.z());
    V=Ns[f.normalIndex(2)];
    normals.push_back(V.x()); normals.push_back(V.y()); normals.push_back(V.z());
    colors.push_back(fabs(V.x())); colors.push_back(fabs(V.y())); colors.push_back(fabs(V.z()));
    TC=texcords[f.texcoordsIndex(2)];
    texCoords.push_back(TC.first);  texCoords.push_back(TC.second);
}

  assert(vertices.size() == 3*numvertices);
  assert(normals.size() == 3*numvertices);
  assert(colors.size() == 3*numvertices);
  assert(texCoords.size() == 2*numvertices);
  
  // Step 2: Create VAO and empty buffers (coords, normals, ...)
  GLWidget& g = *glwidget();
  GLuint VAO;
  g.glGenVertexArrays(1, &VAO);
  VAOs.push_back(VAO);
  g.glBindVertexArray(VAO);
  
  GLuint coordBufferID;
  g.glGenBuffers(1, &coordBufferID);
  coordBuffers.push_back(coordBufferID);
  
  GLuint normalBufferID;
  g.glGenBuffers(1, &normalBufferID);
  normalBuffers.push_back(normalBufferID);
  
  GLuint stBufferID;
  g.glGenBuffers(1, &stBufferID);
  stBuffers.push_back(stBufferID);
  
  GLuint colorBufferID;
  g.glGenBuffers(1, &colorBufferID);
  colorBuffers.push_back(colorBufferID);
  
  numIndices.push_back(numvertices);
  // Step 3: Define VBO data (coords, normals, ...)
  g.glBindBuffer(GL_ARRAY_BUFFER, coordBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
  g.glEnableVertexAttribArray(0);

  g.glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), &normals[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  g.glEnableVertexAttribArray(1);

  g.glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*colors.size(), &colors[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
  g.glEnableVertexAttribArray(2);

  g.glBindBuffer(GL_ARRAY_BUFFER, stBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
  g.glEnableVertexAttribArray(3);
  
  g.glBindBuffer(GL_ARRAY_BUFFER,0);
  g.glBindVertexArray(0);

  //loadTexture();
}

void SSAO::genQuad() {
	GLWidget &g = *glwidget();
  vector<float> quadVertices = {
    // positions        
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 
    1.0f, -1.0f, 0.0f,
  };

  vector<float> texCoords = {
      0.0f, 1.0f,
      0.0f, 0.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
  };
  GLuint quadVBO;
  GLuint quadTex;
  // setup plane VAO
  g.glGenVertexArrays(1, &quadVAO);
  g.glBindVertexArray(quadVAO);
  g.glGenBuffers(1, &quadVBO);
  g.glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quadVertices.size(), &quadVertices[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  g.glEnableVertexAttribArray(0);

  g.glGenBuffers(1, &quadTex);
  g.glBindBuffer(GL_ARRAY_BUFFER, quadTex);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); 
  g.glEnableVertexAttribArray(1);

  g.glBindVertexArray(0);
}

void SSAO::loadTexture() {
  GLWidget& g = *glwidget();

  GLuint textureID;
  g.glActiveTexture(GL_TEXTURE3);
  g.glGenTextures(1, &textureID);
  
  QString filename = QFileDialog::getOpenFileName(0, "Open Diffuse Texture", "/assig/grau-g/Textures", "Image file (*.png *.jpg)");	
	QImage img0(filename);	
	QImage im0 = img0.convertToFormat(QImage::Format_ARGB32).rgbSwapped().mirrored();

  g.glBindTexture(GL_TEXTURE_2D, textureID);
  g.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, im0.width(), im0.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, im0.bits());
  g.glGenerateMipmap(GL_TEXTURE_2D);

  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  textures_diffuse.push_back(textureID);

  g.glBindTexture(GL_TEXTURE_2D, 0);
  g.glActiveTexture(0);
}

void SSAO::setFrameBuffer() {

  GLWidget& g = *glwidget();

  const unsigned int SCR_WIDTH = 1280;
  const unsigned int SCR_HEIGHT = 720;

  gBuf = new QOpenGLFramebufferObject(SCR_WIDTH, SCR_HEIGHT, QOpenGLFramebufferObject::Depth, GL_TEXTURE_2D, GL_RGB16F);
  gBuf->addColorAttachment(SCR_WIDTH, SCR_HEIGHT, GL_RGB16F);
  gBuf->addColorAttachment(SCR_WIDTH, SCR_HEIGHT, GL_RGB);

  unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  g.glDrawBuffers(3, attachments);

  ssaoBuf = new QOpenGLFramebufferObject(SCR_WIDTH, SCR_HEIGHT, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED);

  blurBuf = new QOpenGLFramebufferObject(SCR_WIDTH, SCR_HEIGHT, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED);

  sepBufX = new QOpenGLFramebufferObject(SCR_WIDTH, SCR_HEIGHT, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED);
  sepBufY = new QOpenGLFramebufferObject(SCR_WIDTH, SCR_HEIGHT, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED); 

}

float lerp(float a, float b, float f) {
  return a + f * (b - a);
} 

void SSAO::genKernels() {
  GLWidget& g = *glwidget();
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
  std::default_random_engine generator;
  for (unsigned int i = 0; i < kernelSize; ++i) {
    QVector3D sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
    sample.normalize();
    sample *= randomFloats(generator);
    float scale = float(i) / kernelSize;

    // scale samples s.t. they're more aligned to center of kernel
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    ssaoKernel.push_back(sample);
  }

  std::vector<QVector3D> ssaoNoise;
  for (unsigned int i = 0; i < 16; i++) {
    QVector3D noise(
      randomFloats(generator) * 2.0 - 1.0, 
      randomFloats(generator) * 2.0 - 1.0, 
      0.0f); 
    ssaoNoise.push_back(noise);
  }  

  g.glGenTextures(1, &noiseTexture);
  g.glBindTexture(GL_TEXTURE_2D, noiseTexture);
  g.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  g.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

void SSAO::changeKernelSize() {
  int x;
  do {
  cout << "Please input a positive Integer that is a power of 2 for the NUMBER OF SAMPLES: ";
  cin >> x;
  } while (!(x && (!(x&(x-1)))));
  
  kernelSize = x;
  ssaoKernel.clear();
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
  std::default_random_engine generator;
  for (unsigned int i = 0; i < kernelSize; ++i) {
    QVector3D sample(randomFloats(generator) * 2.0 - 1.0, 0, 0);
    sample.normalize();
    sample *= randomFloats(generator);
    float scale = float(i) / kernelSize;

    // scale samples s.t. they're more aligned to center of kernel
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    ssaoKernel.push_back(sample);
  }
}

void SSAO::changeRadius() {
  double x;
  do {
  cout << "Please input a positive double for the RADIUS: ";
  cin >> x;
  } while (x <= 0);
  
  radius = x;
}

void SSAO::changeBias() {
  double x;
  do {
  cout << "Please input a positive double for the BIAS: ";
  cin >> x;
  } while (x <= 0);
  
  bias = x;
}