#include "bullet.hpp"

Bullet::Bullet(Game *parent, double x, double y, double dx, double dy) 
  : GameObject(parent), x(x), y(y), dx(dx), dy(dy), timeToLive(1000)
{
	static quint16 nextid = 0;

	id = nextid++;
}

void Bullet::step() {
	x += dx;
	y += dy;
    
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
