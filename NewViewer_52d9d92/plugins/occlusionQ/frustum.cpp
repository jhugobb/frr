#include "frustum.h"

Frustum::Frustum() {}

Frustum::Frustum(QVector3D eye, float znear, float zfar, float fov, float aspectRatio, QMatrix4x4 view) {

	float hnear = 2 * tan(fov/2) * znear;
  float wnear = hnear * aspectRatio;
  float hfar = 2 * tan(fov/2) * zfar;
  float wfar = hfar *  aspectRatio;

  QVector3D d = QVector3D(-view(3,0), -view(3,1), -view(3,2));
  d.normalize();

  QVector3D up = QVector3D(view(1,0), view(1,1), view(1,2));
  up.normalize();

  QVector3D right = QVector3D::crossProduct(d, up);
  right.normalize();

	fc = eye + d * zfar;
	ftl = fc + (up * hfar/2) - (right * wfar/2);

	ftr = fc + (up * hfar/2) + (right * wfar/2);
	fbl = fc - (up * hfar/2) - (right * wfar/2);
	fbr = fc - (up * hfar/2) + (right * wfar/2);

	nc = eye + d * znear;

	ntl = nc + (up * hnear/2) - (right * wnear/2);
	ntr = nc + (up * hnear/2) + (right * wnear/2);
	nbl = nc - (up * hnear/2) - (right * wnear/2);
	nbr = nc - (up * hnear/2) + (right * wnear/2);

	pl[TOP].set3Points(ntr,ntl,ftl);
	pl[BOTTOM].set3Points(nbl,nbr,fbr);
	pl[LEFT].set3Points(ntl,nbl,fbl);
	pl[RIGHT].set3Points(nbr,ntr,fbr);
	pl[NEARP].set3Points(ntl,ntr,nbr);
	pl[FARP].set3Points(ftr,ftl,fbl);
}

Frustum::~Frustum() {}

int Frustum::sphereInFrustum(QVector3D p, float radius) {

	int result = INSIDE;
	float distance;

	for(int i=0; i < 6; i++) {
		distance = pl[i].distance(p);
		if (distance < -radius)
			return OUTSIDE;
		else if (distance < radius)
			result =  INTERSECT;
	}
	return(result);

}