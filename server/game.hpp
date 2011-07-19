#ifndef GAME_HPP
#define GAME_HPP

#include <QObject>
#include <QUdpSocket>
#include <QHash>
#include <QDateTime>
#include <QTimer>

class Participant;

class Game : public QObject {
    Q_OBJECT

	quint32 gameTicks;
	QDateTime gameTime;
	QTimer timer, gcTimer;

	QUdpSocket s;
	QHash<QString, Participant*> p;

	Participant* acquireParticipant(QHostAddress srcHost, quint16 port);

	void sendUpdates();
	void step();

public:
    explicit Game(QObject *parent = 0);

signals:

public slots:

private slots:
	void incoming();
	void timerSlot();
	void disconnectStaleClients();
};

#endif // GAME_HPP
