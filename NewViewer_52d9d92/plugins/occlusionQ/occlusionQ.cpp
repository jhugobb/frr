// GLarena, a plugin based platform to teach OpenGL programming
// © Copyright 2012-2018, ViRVIG Research Group, UPC, https://www.virvig.eu
// 
// This file is part of GLarena
//
// GLarena is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "occlusionQ.h"
#include "frustum.h"
#include <string.h>

#define copies 40

OccQ::~OccQ() {
  cleanUp();
}

void OccQ::onSceneClear() {
  cleanUp();
}

void OccQ::updateFPS() {
  current = previous;
  previous = 0;
  glwidget()->update();
}

void OccQ::keyPressEvent(QKeyEvent*e) 
{
	if (e->key()==Qt::Key_O)
  {
    useVFC = false;
		if (useOcc) 
    {
			useOcc = false;
		}
		else 
    {
			useOcc = true;
		}
	}

  if (e->key()==Qt::Key_V)
  {
    useOcc = false;
		if (useVFC) 
    {
			useVFC = false;
		}
		else 
    {
			useVFC = true;
		}
	}
}

void OccQ::cleanUp() {
  GLWidget &g = *glwidget();
  g.makeCurrent();
  g.glDeleteBuffers(coordBuffers.size(),  &coordBuffers[0]);
  g.glDeleteBuffers(normalBuffers.size(), &normalBuffers[0]);
  g.glDeleteBuffers(stBuffers.size(),  &stBuffers[0]);
  g.glDeleteBuffers(colorBuffers.size(),  &colorBuffers[0]);
  g.glDeleteBuffers(bboxBuffers.size(), &bboxBuffers[0]);
  g.glDeleteVertexArrays(VAOs.size(), &VAOs[0]);
  coordBuffers.clear();
  normalBuffers.clear();
  stBuffers.clear();
  colorBuffers.clear();
  bboxBuffers.clear();
  VAOs.clear();
  numIndices.clear();
  translation.clear();
}

void OccQ::onPluginLoad() {

  GLWidget & g = *glwidget();
  g.makeCurrent();

    bboxPoints = {
    1, 1, 1,
    0, 1, 1,
    1, 0, 1,
    0, 0, 1,
    1, 0, 0,
    0, 0, 0,
    1, 1, 0,
    0, 1, 0,
    1, 1, 1,
    0, 1, 1,
    0, 1, 1,
    0, 1, 0,
    0, 0, 1,
    0, 0, 0,
    1, 0, 1,
    1, 0, 0,
    1, 1, 1,
    1, 1, 0
  };

  vs = new QOpenGLShader(QOpenGLShader::Vertex, this);
  vs->compileSourceFile(g.getPluginPath()+"/../occlusionQ/occlusionQ.vert");
  cout << "VS log:" << vs->log().toStdString() << endl;

  fs = new QOpenGLShader(QOpenGLShader::Fragment, this);
  fs->compileSourceFile(g.getPluginPath()+"/../occlusionQ/occlusionQ.frag");
  cout << "FS log:" << fs->log().toStdString() << endl;

  program = new QOpenGLShaderProgram(this);
  program->addShader(vs);
  program->addShader(fs);
  program->link();
  cout << "Link log:" << program->log().toStdString() << endl;

  // Initialises VAOs and VBOs
  addVBO();

  // Calculate BBox scaling
  calBboxScale();
  
  // Initialises FPS counter
  current=previous=0;
  QTimer *timer=new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateFPS()));
  timer->start(1000);

  useOcc = false;
  useBbox = false;
  useVFC = false;
}

bool OccQ::drawObject(int i, QMatrix4x4 MVP) {
  GLWidget &g = *glwidget();
  g.makeCurrent();

  if (useBbox) program->setUniformValue("scale", scale);
  else program->setUniformValue("scale", QMatrix4x4());
  program->setUniformValue("modelViewProjectionMatrix", MVP);
  g.glBindVertexArray(VAOs[0]);

  if (useBbox) {
    // We draw the Bounding Box
    g.glDrawArrays(GL_TRIANGLE_STRIP, 0, 18);
  } else {
    // We draw the whole object
    g.glDrawArrays(GL_TRIANGLES, 0, numIndices[0]);
  }
  g.glBindVertexArray(0);
  return true;
}

bool OccQ::drawScene() {
  GLWidget &g = *glwidget();
  g.makeCurrent();

  QMatrix4x4 view = camera()->projectionMatrix() * camera()->viewMatrix();
  QMatrix4x4 MVP;
  if (useOcc) {

    g.glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    g.glDepthMask(GL_FALSE);

    GLuint queries[copies*copies];
    g.glGenQueries(copies*copies, queries);
    useBbox = true;
    for(unsigned int i=0; i<copies*copies; i++) {
      MVP = QMatrix4x4(view);
      MVP.translate(translation[i]);
      g.glBeginQuery(GL_ANY_SAMPLES_PASSED, queries[i]);
      //draw BB 
      drawObject(i, MVP);
      g.glEndQuery(GL_ANY_SAMPLES_PASSED);
    }
    useBbox = false;

    int u = copies*copies*3/4;
    GLint available;
    do {
      g.glGetQueryObjectiv(queries[u],GL_QUERY_RESULT_AVAILABLE, &available);
    } while (!available);

    g.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    g.glDepthMask(GL_TRUE);

    GLuint result;
    for (unsigned int i = 0; i < copies*copies; i++) {
      g.glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT, &result);
      if (result != 0){
        MVP = QMatrix4x4(view);
        MVP.translate(translation[i]);
        drawObject(i, MVP);
      }
       
    }
  } else if (useVFC) {
    QVector4D center = QVector4D((scene()->objects()[0]).boundingBox().center(),1);
    QVector4D max = QVector4D((scene()->objects()[0]).boundingBox().max(), 1);
    for(unsigned int i=0; i<copies*copies; i++) { // for each buffer (that is, for each object)
      MVP = QMatrix4x4(view);
      MVP.translate(translation[i]);
      QVector3D c = (center + translation[i]).toVector3D();

      QVector3D m = (max + translation[i]).toVector3D();

      int result = frustum.sphereInFrustum(c, c.distanceToPoint(m));
      if (result != 0) {
        drawObject(i, MVP);
      }
    } 
  } else {
    for(unsigned int i=0; i<copies*copies; i++) {
      MVP = QMatrix4x4(view);
      MVP.translate(translation[i]);
      drawObject(i, MVP);
    }
  }

  return true;
}

void OccQ::preFrame() {
  program->bind();
  camera()->setZfar(200.0);
  calculateFrustum();
  program->setUniformValue("bboxMax", scene()->objects()[0].boundingBox().max());
  program->setUniformValue("bboxMin", scene()->objects()[0].boundingBox().min());

}

void OccQ::postFrame() {
  painter.begin(glwidget());
  painter.drawText(10,15, QString("%0 fps").arg(current));
  painter.drawText(10,35, QString("Occlusion: %0").arg(useOcc));
  painter.drawText(10,55, QString("V. F. Culling: %0").arg(useVFC));
  painter.end();
  
  program->release();
  ++previous;
}

void OccQ::onObjectAdd() {
  addVBO();
}

void OccQ::addVBO() {
  const Object& obj = scene()->objects()[0];
  unsigned int numvertices = obj.faces().size()*3;  // it's all triangles...
  vector<float> vertices; // (x,y,z)    Final size: 9*number of triangles
  vector<float> normals;  // (nx,ny,nz) Final size: 9*number of triangles
  vector<float> colors;   // (r, g, b)  Final size: 9*number of triangles
  vector<float> texCoords;// (s, t)     Final size: 6*number of triangles
  auto verts = obj.vertices();
  auto Ns = obj.vertNormals();
  auto texcords = obj.vertTexCoords();
  
  QVector3D t;
  double maxX = obj.boundingBox().max()[0]*2;
  double maxZ = obj.boundingBox().max()[2]*2;
  for (int i=0; i<copies; i++) {
    for (int j=0; j<copies; j++) {
      t =  QVector3D(i*maxX, 0, j*maxZ);
      translation.push_back(t);
    }
  }

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

  GLuint bboxBufferID;
  g.glGenBuffers(1, &bboxBufferID);
  bboxBuffers.push_back(bboxBufferID);
  
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

  g.glBindBuffer(GL_ARRAY_BUFFER, bboxBufferID);
  g.glBufferData(GL_ARRAY_BUFFER, sizeof(float)*bboxPoints.size(), &bboxPoints[0], GL_STATIC_DRAW);
  g.glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
  g.glEnableVertexAttribArray(4);
  
  g.glBindBuffer(GL_ARRAY_BUFFER,0);
  g.glBindVertexArray(0);
}


void OccQ::calculateFrustum() {

  QVector3D eye = camera()->getObs();
  float znear = camera()->znear();
  float zfar = camera()->zfar();
  float fov = camera()->fov();
  float aspectRatio = camera()->aspectRatio();
  QMatrix4x4 view = camera()->viewMatrix();

  frustum = Frustum(eye, znear, zfar, fov, aspectRatio, view);

}
void OccQ::calBboxScale() {
  const Object& obj = scene()->objects()[0];
  Box box = obj.boundingBox();
  scale = QMatrix4x4();
  scale(0,0) = box.max().x() - box.min().x();
  scale(1,1) = box.max().y() - box.min().y();
  scale(2,2) = box.max().z() - box.min().z();
  scale(3,3) = 1;
  for (int i = 0; i < 18; i+=3) {
    QVector3D point = QVector3D(bboxPoints[i], bboxPoints[i+1], bboxPoints[i+2]);
    QVector4D v = scale*QVector4D(point-QVector3D(0.5,0.5,0.5), 1);
    bboxPoints[i] = point.x() + box.center().x();
    bboxPoints[i+1] = point.y() + box.center().y();
    bboxPoints[i+2] = point.z() + box.center().z();
  }
}

