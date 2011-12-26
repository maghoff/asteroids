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


	double get_x() const { return x; }
	double get_y() const { return y; }
	double get_dx() const { return dx; }
	double get_dy() const { return dy; }
	quint8 get_id() const { return id; }


private:
	struct {
		quint8 r, g, b;
	} color;
	
	quint8 id;

	double x, dx;
	double y, dy;
	double ang, dang;

	bool engine;
	bool is_firing;

	long weapon_cooldown;

	QString name;

    Game *game();

signals:

public slots:

};

#endif // PARTICIPANT_HPP
