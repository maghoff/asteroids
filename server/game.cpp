#include <stdexcept>
#include <QDebug>
#include <QSet>
#include "game.hpp"
#include "gameobject.hpp"
#include "participant.hpp"

Game::Game(QObject *parent) :
    QObject(parent)
{
	connect(&s, SIGNAL(readyRead()), this, SLOT(incoming()));

	bool ok = s.bind(50005);
	if (!ok) {
		qDebug() << "Unable to bind to port";
		throw std::runtime_error("Unable to bind to port");
	}

	connect(&timer, SIGNAL(timeout()), this, SLOT(timerSlot()));
	timer.setInterval(10);

	connect(&gcTimer, SIGNAL(timeout()), this, SLOT(disconnectStaleClients()));
	gcTimer.setInterval(1000);

	gameTime = QDateTime::currentDateTime();
	gameTicks = 0;
	nextSendShipInfo = 0;

	qDebug() << "Engine is suspended";
}

Participant* Game::acquireParticipant(QHostAddress srcHost, quint16 srcPort) {
	QString id;
	QTextStream ss(&id);
	ss << srcHost.toString() << ':' << srcPort;

	QHash<QString, Participant*>::const_iterator i = p.find(id);
	if (i == p.end()) {
		qDebug() << id << ": New participant";

		if (!timer.isActive()) {
			qDebug() << "Taking engine out of suspend";
			leaveSuspend();
		}

		i = p.insert(id, new Participant(this, srcHost, srcPort));
	}

	return *i;
}

const quint8 MSG_PING = 17;

void Game::incoming() {
	qint64 pendingSize = s.pendingDatagramSize();
	std::vector<char> d(pendingSize);

	QHostAddress srcHost;
	quint16 srcPort;

	qint64 sz = s.readDatagram(d.data(), d.size(), &srcHost, &srcPort);

	if (sz != pendingSize) {
		qDebug() << "ERROR while reading incoming datagram. Sizes did not match. Disregarding datagram.";
		return;
	}

	if (d[0] == MSG_PING) {
		s.writeDatagram(d.data(), d.size(), srcHost, srcPort);
	}

	Participant* p = acquireParticipant(srcHost, srcPort);
	p->incoming(d);
}

void Game::sendToAll(QByteArray& datagram) {
	typedef QHash<QString, Participant*>::const_iterator iter;

	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
		qint64 w = s.writeDatagram(datagram, (*i)->host, (*i)->port);
		if (w != datagram.size()) {
			if (w == -1) {
				qDebug() << "ERROR writing datagram";
			} else {
				qDebug() << "ERROR writing datagram. Sizes did not match (Wrote " << w << "b of " << datagram.size() << "b)";
			}
		}
	}
}
const quint8 MSG_GAME_STATUS = 0x07;
const quint8 MSG_SHIP_INFO = 0x3b;

void Game::sendObjectUpdates() {
	typedef QHash<QString, Participant*>::const_iterator iter;
	typedef QList<GameObject*>::const_iterator liter;

	QByteArray datagram;
	QDataStream ds(&datagram, QIODevice::WriteOnly);
	ds.setFloatingPointPrecision(QDataStream::SinglePrecision);

	ds << MSG_GAME_STATUS;
	ds << (quint32)(gameTicks);

	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
        (*i)->serializeStatus(ds);
	}

	for (liter i = go.begin(), e = go.end(); i != e; ++i) {
        (*i)->serializeStatus(ds);
	}

	sendToAll(datagram);
}

void Game::sendShipInfo() {
	typedef QHash<QString, Participant*>::const_iterator iter;

	QByteArray datagram;
	QDataStream ds(&datagram, QIODevice::WriteOnly);
	ds.setFloatingPointPrecision(QDataStream::SinglePrecision);

	ds << MSG_SHIP_INFO;

	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
        (*i)->serializeShipInfo(ds);
	}

	sendToAll(datagram);
}

void Game::step() {
	typedef QHash<QString, Participant*>::iterator iter;
	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
		(*i)->step();
	}
	typedef QList<GameObject*>::iterator liter;
	for (liter i = go.begin(), e = go.end(); i != e; ++i) {
		(*i)->step();
	}

}

void Game::enterSuspend() {
	timer.stop();
	gcTimer.stop();
	engineSuspended();
}

void Game::leaveSuspend() {
	gameTime = QDateTime::currentDateTime();
	gameTicks = gameTime.toMSecsSinceEpoch();
	timer.start();
	gcTimer.start();
}

void Game::timerSlot() {
	for (; gameTime < QDateTime::currentDateTime(); gameTime = gameTime.addMSecs(1)) {
		step();
		gameTicks++;
	}

	sendObjectUpdates();

	if (gameTicks >= nextSendShipInfo)
	{
		sendShipInfo();
		nextSendShipInfo = gameTicks + 1000;
	}
}

void Game::disconnectStaleClients() {
	typedef QHash<QString, Participant*>::const_iterator iter;
	const int timeoutLimitSecs = 5;

	QDateTime currentTime = QDateTime::currentDateTime();
	QSet<QString> stale;

	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
		QDateTime lastSeen = (*i)->lastSeen;
		if (lastSeen.isValid() && (lastSeen.secsTo(currentTime) > timeoutLimitSecs)) {
			stale.insert(i.key());
		}
	}

	for (QSet<QString>::const_iterator i = stale.begin(), end = stale.end(); i != end; ++i) {
		qDebug() << (*i) << "Kicking out stale client";
		delete p.value(*i);
		p.remove(*i);
	}

	if (p.size() == 0) {
		qDebug() << "No clients connected; suspending engine";
		enterSuspend();
	}
}

void Game::add(GameObject *o) {
    go.push_front(o);
}

void Game::remove(GameObject *o) {
    go.removeOne(o);
}
