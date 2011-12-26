#include <cmath>
#include "spacemath.hpp"

void cropToSpace(double &x, double &y) {

	const double spaceWidth = 640. + 2. * 15.;
	const double spaceHeight = 400. + 2. * 15.;

	x = fmod(x + spaceWidth / 2., spaceWidth) - spaceWidth / 2.;
	if (x < -spaceWidth / 2.) x += spaceWidth;

	y = fmod(y + spaceHeight / 2., spaceHeight) - spaceHeight / 2.;
	if (y < -spaceHeight / 2.) y += spaceHeight;

}

static double sqr(double x) { return x*x; }

double dist(double x1, double y1, double x2, double y2) {
	return sqrt(sqr(x1 - x2) + sqr(y1 - y2));
}
