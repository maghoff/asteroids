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

