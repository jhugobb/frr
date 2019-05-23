#ifndef _SSAO_H
#define _SSAO_H

#include "plugin.h" 
#include <cmath>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <random>

class SSAO: public QObject, public Plugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "Plugin") 
	Q_INTERFACES(Plugin)

  public:
	 ~SSAO();
	 void onPluginLoad();
	 void preFrame();
	 void postFrame();
	 void onSceneClear();
	 void onObjectAdd();
	 void cleanUp();
	 bool drawObjectFB(unsigned int);

	 bool paintGL();

	 void keyPressEvent(QKeyEvent *);
	 void mouseMoveEvent(QMouseEvent *);

	public slots:
    void updateFPS();
  private:
	// add private methods and attributes here
		void addVBO(unsigned int);
		void loadTexture();
		void drawQuad();
		void regularAO();
		void separableAOY();
		void separableAOX();
		void setQuad();
		void setFrameBuffer();
		void genKernels();
		void genQuad();
		void setUpShaders();
		void changeKernelSize();
		void changeRadius();
		void changeBias();

		QOpenGLShaderProgram* ssaoProgram;
		QOpenGLShaderProgram* gprogram;
		QOpenGLShaderProgram* lightProgram;
		QOpenGLShaderProgram* nossaoProgram;
		QOpenGLShaderProgram* sepssaoXProgram;
		QOpenGLShaderProgram* sepssaoYProgram;
		QOpenGLShaderProgram* blurProgram;
    QOpenGLShader* ssaovs;
    QOpenGLShader* ssaofs;  
		QOpenGLShader* nossaofs;
		QOpenGLShader* sepssaoXfs;
		QOpenGLShader* sepssaoYfs;
		QOpenGLShader* blurfs;
		QOpenGLShader* gvs;
		QOpenGLShader* gfs;
		QOpenGLShader* lightvs;
		QOpenGLShader* lightfs;
    GLuint textureId;
    Camera lightCamera;

		QVector3D lightPos;

		bool useAO;
		bool useBlur;
		bool useNormal;

		float current;
		float previous;
		QPainter painter;

		// We will create a VBO for each object in the scene
    vector<GLuint> VAOs;          // ID of VAOs
    vector<GLuint> coordBuffers;  // ID of vertex coordinates buffer 
    vector<GLuint> normalBuffers; // ID of normal components buffer 
    vector<GLuint> stBuffers;     // ID of (s,t) buffer 
    vector<GLuint> colorBuffers;  // ID of color buffer  
    vector<GLuint> numIndices;    // Size (number of indices) in each index buffer
		
		vector<GLuint> textures_diffuse;
		GLuint gBuffer; 			// ID of G-Buffers
		GLuint gPosition;
		GLuint gNormal;
		GLuint gAlbedoSpec;
		GLuint rboDepth;
		GLuint quadVAO;

		QOpenGLFramebufferObject *gBuf;
		QOpenGLFramebufferObject *ssaoBuf;
		QOpenGLFramebufferObject *blurBuf;
		QOpenGLFramebufferObject *sepBufX;
		QOpenGLFramebufferObject *sepBufY;

		// SSAO
		GLuint noiseTexture;
		std::vector<QVector3D> ssaoKernel;
		unsigned int kernelSize;
		float radius;
		float bias;
};

#endif
