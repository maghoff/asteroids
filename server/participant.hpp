#ifndef PARTICIPANT_HPP
#define PARTICIPANT_HPP

#include <QObject>
#include <QHostAddress>
#include <QTime>
#include <vector>

class Participant : public QObject {
    Q_OBJECT

public:

	QTime lastSeen;

	QHostAddress host;
	quint16 port;

	explicit Participant(QObject *parent, QHostAddress host, quint16 port);
	~Participant();

	void incoming(const std::vector<char>&);

	struct {
		quint8 r, g, b;
	} color;

	double x, dx;
	double y, dy;
	double ang, dang;

	bool engine;

	void step();

signals:

public slots:

};

#endif // PARTICIPANT_HPP
