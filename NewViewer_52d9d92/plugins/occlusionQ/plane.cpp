#include "plane.h"

Plane::Plane() {}

Plane::~Plane() {}

void Plane::set3Points( QVector3D v1,  QVector3D v2,  QVector3D v3) {


	QVector3D aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = aux2 * aux1;

	normal.normalize();
	point = QVector3D(v2);
	d = -(QVector3D::dotProduct(normal, point));
}

float Plane::distance(QVector3D p) {

	return (d + QVector3D::dotProduct(normal, p));
}