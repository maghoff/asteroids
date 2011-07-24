#ifndef PARTICIPANT_HPP
#define PARTICIPANT_HPP

#include <QObject>
#include <QHostAddress>
#include <QDateTime>
#include <QString>
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
	void serializeShipInfo(QDataStream& ds);


private:
	struct {
		quint8 r, g, b;
	} color;
	
	quint8 id;

	double x, dx;
	double y, dy;
	double ang, dang;

	bool engine;

	QString name;

    Game *game();

signals:

public slots:

};

#endif // PARTICIPANT_HPP
