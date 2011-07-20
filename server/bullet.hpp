#ifndef BULLET_HPP
#define BULLET_HPP

#include <QObject>
#include <QHostAddress>
#include <QDateTime>
#include <vector>
#include "gameobject.hpp"

class Bullet : public GameObject {
    Q_OBJECT

public:

	explicit Bullet(QObject *parent, double x, double y, double dx, double dy);

	virtual void step();

    virtual void serializeStatus(QDataStream& ds);

private:
    double x, y, dx, dy;

signals:

public slots:

};

#endif // BULLET_HPP
