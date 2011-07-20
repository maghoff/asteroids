#include "bullet.hpp"

Bullet::Bullet(Game *parent, double x, double y, double dx, double dy) 
  : GameObject(parent), x(x), y(y), dx(dx), dy(dy), timeToLive(1000)
{}

void Bullet::step() {
	x += dx;
	y += dy;
    
    if (timeToLive-- == 0) {
        game()->remove(this);  
    }
}

const quint8 OBJ_BULLET = 0x08;

void Bullet::serializeStatus(QDataStream& ds) {
	ds << OBJ_BULLET;
	ds << x << dx;
	ds << y << dy;
}

Game* Bullet::game() {
    return (Game*) parent();
}
