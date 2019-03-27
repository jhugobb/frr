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

#ifndef _OCCQ_H
#define _OCCQ_H

#include "plugin.h"
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <cassert>
#include <cmath>

class OccQ : public QObject, public Plugin
 {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "Plugin")   
  Q_INTERFACES(Plugin)

  public:
    void onPluginLoad();
    bool drawObject(int i);
    bool drawScene();
    void preFrame();
    void postFrame();
    void onObjectAdd();
    void keyPressEvent(QKeyEvent*e);
    void cleanUp();
    void calculateBBox();
    void onSceneClear();
    ~OccQ();
      
  public slots:
    void updateFPS();

  private:
    QOpenGLShaderProgram* program;
    QOpenGLShader *fs, *vs; 

    double current;
    double previous;

    vector<QVector3D> translation;
    QMatrix4x4 bboxMatrix;

    bool useOcc;
    bool useBbox;
    QPainter painter;

    vector<vector<GLfloat>> bboxes;

    void addVBO(unsigned int currentObject, QVector3D translation);

    // We will create a VBO for each object in the scene
    vector<GLuint> VAOs;          // ID of VAOs
    vector<GLuint> coordBuffers;  // ID of vertex coordinates buffer 
    vector<GLuint> normalBuffers; // ID of normal components buffer 
    vector<GLuint> stBuffers;     // ID of (s,t) buffer 
    vector<GLuint> colorBuffers;  // ID of color buffer  
    vector<GLuint> numIndices;    // Size (number of indices) in each index buffer
    vector<GLuint> bboxVAOs;
    vector<GLuint> bboxBuffers;
};
 
 #endif

