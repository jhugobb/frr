#ifndef _SSAO_H
#define _SSAO_H

#include "plugin.h" 
#include <cmath>
#include <QOpenGLFramebufferObject>

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
  private:
	// add private methods and attributes here
		void addVBO(unsigned int);
		void loadTexture();
		void drawQuad();
		void setQuad();
		void setFrameBuffer();

		QOpenGLShaderProgram* program;
		QOpenGLShaderProgram* gprogram;
    QOpenGLShader* vs;
    QOpenGLShader* fs;  
		QOpenGLShader* gvs;
		QOpenGLShader* gfs;
    GLuint textureId;
    Camera lightCamera;

		float radius;

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

		QOpenGLFramebufferObject *fbo;
};

#endif
