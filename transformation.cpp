#include "transformation.h"

Transformation::Transformation() {
	m = Matrix();
	minvt = Matrix();
}

Vector Transformation::transform(Vector v) {
	// need help, will the result be a 1x4 vector or 1x3 vector? since currently the vector class only supports 1x3 vectors
}