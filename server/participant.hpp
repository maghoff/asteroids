#ifndef PARTICIPANT_HPP
#define PARTICIPANT_HPP

#include <QObject>
#include <QHostAddress>
#include <QDateTime>
#include <vector>
#include "gameobject.hpp"
#include "game.hpp"

class Participant : public GameObject {
    Q_OBJECT

public:

	QDateTime lastSeen;

	QHostAddress host;
	quint16 port;

	explicit Participant(Game *parent, QHostAddress host, quint16 port);
	~Participant();

	void incoming(const std::vector<char>&);


	virtual void step();

    virtual void serializeStatus(QDataStream& ds);

private:
	struct {
		quint8 r, g, b;
	} color;

	double x, dx;
	double y, dy;
	double ang, dang;

	bool engine;

    Game *game();

signals:

public slots:

};

#endif // PARTICIPANT_HPP
