#ifndef EXPLOSION_HPP
#define EXPLOSION_HPP

#include "gameobject.hpp"

class Game;

class Explosion : public GameObject {
public:
	Explosion(Game* game, double x, double y, double dx, double dy);

	void step();

	void serializeStatus(QDataStream& ds);

private:
	Game* game;
	double x, y, dx, dy;
	int timeToLive;
	quint16 id;
};

#endif // EXPLOSION_HPP
