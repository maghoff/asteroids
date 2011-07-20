#ifndef GAME_HPP
#define GAME_HPP

#include <QObject>
#include <QUdpSocket>
#include <QHash>
#include <QDateTime>
#include <QList>
#include <QTimer>
#include "gameobject.hpp"

class Participant;

class Game : public QObject {
    Q_OBJECT

	quint32 gameTicks;
	QDateTime gameTime;
	QTimer timer, gcTimer;

	QUdpSocket s;
	QHash<QString, Participant*> p;
    QList<GameObject*> go;

	Participant* acquireParticipant(QHostAddress srcHost, quint16 port);

	void sendUpdates();
	void step();


public:
    explicit Game(QObject *parent = 0);

    void add(GameObject *o);
    void remove(GameObject *o);

signals:

public slots:

private slots:
	void incoming();
	void timerSlot();
	void disconnectStaleClients();
};

#endif // GAME_HPP
