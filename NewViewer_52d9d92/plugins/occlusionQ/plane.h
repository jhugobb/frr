#ifndef _PLANE_H
#define _PLANE_H

#include "plugin.h"

class Plane {
  public:

    QVector3D normal,point;
    float d;

    Plane();
    ~Plane();

    void set3Points( QVector3D v1,  QVector3D v2,  QVector3D v3);
    float distance(QVector3D p);
};


#endif