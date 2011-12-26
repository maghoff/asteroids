#include "game.hpp"
#include "explosion.hpp"

Explosion::Explosion(Game* game_, double x_, double y_, double dx_, double dy_) :
	GameObject(game_),
	game(game_),
	x(x_),
	y(y_),
	dx(dx_),
	dy(dy_)
{
	static quint16 nextid = 0;
	id = nextid++;

	timeToLive = 2000;
}

void Explosion::step() {
	if (--timeToLive == 0) {
		game->remove(this);
	}
}

const quint8 OBJ_EXPLOSION = 0x0b;

void Explosion::serializeStatus(QDataStream& ds) {
	ds << OBJ_EXPLOSION;
	ds << id;
	ds << x << dx;
	ds << y << dy;
}
