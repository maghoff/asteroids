#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <QObject>
#include <QHostAddress>
#include <QDateTime>
#include <vector>

class GameObject : public QObject {
    Q_OBJECT

public:
    GameObject(QObject *parent);

	virtual void step() = 0;

    virtual void serializeStatus(QDataStream& ds) = 0;

signals:

public slots:

};

#endif // GAMEOBJECT_HPP
