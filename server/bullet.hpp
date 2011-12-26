#ifndef BULLET_HPP
#define BULLET_HPP

#include <QObject>
#include <QHostAddress>
#include <QDateTime>
#include <vector>
#include "gameobject.hpp"
#include "game.hpp"

class Bullet : public GameObject {
    Q_OBJECT

public:

	Bullet(Game *parent, double x, double y, double dx, double dy, quint8 playerId);

	virtual void step();

    virtual void serializeStatus(QDataStream& ds);

private:
    double x, y, dx, dy;
    int timeToLive;
	quint16 id;
	quint8 playerId;

    Game *game();

signals:

public slots:

};

#endif // BULLET_HPP
