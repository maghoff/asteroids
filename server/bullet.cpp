#include "explosion.hpp"
#include "participant.hpp"
#include "spacemath.hpp"
#include "bullet.hpp"

Bullet::Bullet(Game *parent, double x, double y, double dx, double dy, quint8 playerId_) :
	GameObject(parent), x(x), y(y), dx(dx), dy(dy), timeToLive(1000), playerId(playerId_)
{
	static quint16 nextid = 0;

	id = nextid++;
}

void Bullet::step() {
	x += dx;
	y += dy;

	typedef QHash<QString, Participant*>::iterator iter;
	for (iter i = game()->p.begin(), e = game()->p.end(); i != e; ++i) {
		const Participant& p = **i;
		if (dist(p.get_x(), p.get_y(), x, y) < 24 && playerId != p.get_id()) {
			game()->add(new Explosion(game(), p.get_x(), p.get_y(), p.get_dx(), p.get_dy()));
			game()->remove(this);
			return;
		}
	}

    if (timeToLive-- == 0) {
		game()->remove(this);
    }
}

const quint8 OBJ_BULLET = 0x0a;

void Bullet::serializeStatus(QDataStream& ds) {
	ds << OBJ_BULLET;
	ds << id;
	ds << x << dx;
	ds << y << dy;
}

Game* Bullet::game() {
    return (Game*) parent();
}
