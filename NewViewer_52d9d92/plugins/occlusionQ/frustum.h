#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "plane.h"
#include "plugin.h"
#include <math.h>

class Frustum {
  public:

    Plane pl[6];

    QVector3D nc, fc, ntl,ntr,nbl,nbr,ftl,ftr,fbl,fbr;
    float nearD, farD, ratio, angle,tang;
    float nw,nh,fw,fh;

    Frustum();
    Frustum(QVector3D eye, float znear, float zfar, float fov, float aspectRatio, QMatrix4x4 view);
    ~Frustum();

    void setCamInternals(float angle, float ratio, float nearD, float farD);
    void setCamDef(QVector3D p, QVector3D l, QVector3D u);
    int sphereInFrustum(QVector3D p, float radius);

  private:

    enum {
      TOP = 0,
      BOTTOM,
      LEFT,
      RIGHT,
      NEARP,
      FARP
    };

    enum {
      OUTSIDE, 
      INTERSECT, 
      INSIDE
    };


};


#endif