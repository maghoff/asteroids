#include "bullet.hpp"

Bullet::Bullet(QObject *parent, double x, double y, double dx, double dy) 
  : GameObject(parent), x(x), y(y), dx(dx), dy(dy) 
{}

void Bullet::step() {

}

const quint8 OBJ_BULLET = 0x08;

void Bullet::serializeStatus(QDataStream& ds) {
    
}
