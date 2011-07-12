#include <stdexcept>
#include <QDebug>
#include <QSet>
#include "game.hpp"
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
	timer.start();

	connect(&gcTimer, SIGNAL(timeout()), this, SLOT(disconnectStaleClients()));
	gcTimer.setInterval(1000);
	gcTimer.start();

	gameTime = QTime::currentTime();
	gameTicks = 0;
}

Participant* Game::acquireParticipant(QHostAddress srcHost, quint16 srcPort) {
	QString id;
	QTextStream ss(&id);
	ss << srcHost.toString() << ':' << srcPort;

	QHash<QString, Participant*>::const_iterator i = p.find(id);
	if (i == p.end()) {
		qDebug() << id << ": New participant";

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

const quint8 MSG_GAME_STATUS = 0x07;
const quint8 OBJ_SHIP = 0x07;

void Game::sendUpdates() {
	typedef QHash<QString, Participant*>::const_iterator iter;

	QByteArray datagram;
	QDataStream ds(&datagram, QIODevice::WriteOnly);
	ds.setFloatingPointPrecision(QDataStream::SinglePrecision);

	ds << MSG_GAME_STATUS;
	ds << (quint32)(gameTicks);

	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
		Participant& o = **i;
		//ds << size in bytes, for skipping?;
		ds << OBJ_SHIP;
		ds << o.color.r << o.color.g << o.color.b;
		ds << o.x << o.dx;
		ds << o.y << o.dy;
		ds << o.ang << o.dang;
		ds << (quint8)(o.engine ? 1 : 0);
	}

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

void Game::step() {
	typedef QHash<QString, Participant*>::iterator iter;
	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
		(*i)->step();
	}
}

void Game::timerSlot() {
	for (; gameTime < QTime::currentTime(); gameTime = gameTime.addMSecs(1)) {
		step();
		gameTicks++;
	}

	sendUpdates();
}

void Game::disconnectStaleClients() {
	typedef QHash<QString, Participant*>::const_iterator iter;
	const int timeoutLimit = 5000;

	QTime currentTime = QTime::currentTime();
	QSet<QString> stale;

	for (iter i = p.begin(), e = p.end(); i != e; ++i) {
		QTime lastSeen = (*i)->lastSeen;
		if (lastSeen.isValid() && (lastSeen.msecsTo(currentTime) > timeoutLimit)) {
			stale.insert(i.key());
		}
	}

	for (QSet<QString>::const_iterator i = stale.begin(), end = stale.end(); i != end; ++i) {
		qDebug() << (*i) << "Kicking out stale client";
		delete p.value(*i);
		p.remove(*i);
	}
}