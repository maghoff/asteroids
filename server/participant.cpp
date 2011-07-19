#include <cmath>
#include "participant.hpp"

Participant::Participant(QObject *parent, QHostAddress host_, quint16 port_) :
	QObject(parent),
	host(host_),
	port(port_)
{
	int col = 7 * (rand() / (double)RAND_MAX) + 1;
	color.r = (col & 0x01) ? 0xFF : 0x00;
	color.g = (col & 0x02) ? 0xFF : 0x00;
	color.b = (col & 0x04) ? 0xFF : 0x00;

	x = 0; dx = 0;
	y = 0; dy = 0;
	ang = 0; dang = 0;

	engine = false;
}

Participant::~Participant() {
}

const quint8 UP_BITMASK = 0x01;
const quint8 DOWN_BITMASK = 0x02;
const quint8 LEFT_BITMASK = 0x04;
const quint8 RIGHT_BITMASK = 0x08;

const quint8 MSG_CONTROL_STATE = 0x02;

void Participant::incoming(const std::vector<char>& data) {
	lastSeen = QTime::currentTime();

	QByteArray ba(data.data(), data.size());
	QDataStream ds(&ba, QIODevice::ReadOnly);

	quint8 msgType;

	ds >> msgType;

	if (msgType == MSG_CONTROL_STATE) {
		quint32 time;
		quint8 controlBits;

		ds >> /*time >>*/ controlBits;

		bool up = controlBits & UP_BITMASK;
		bool down = controlBits & DOWN_BITMASK;
		bool left = controlBits & LEFT_BITMASK;
		bool right = controlBits & RIGHT_BITMASK;

		dang = (left ? 1 : 0) + (right ? -1 : 0);
		dang *= 0.005;
		engine = up && !down;

		if (down) {
			dx = dy = 0;
		}
	}
}

void Participant::step() {
	if (engine) {
		const double acc = 0.001;
		dx += acc * cos(ang);
		dy += acc * sin(ang);

		const double speed = sqrt(dx*dx + dy*dy);
		const double maxSpeed = 0.3;
		if (speed > maxSpeed) {
			dx *= maxSpeed / speed;
			dy *= maxSpeed / speed;
		}
	}

	const double damp = 0.999;
	dx *= damp;
	dy *= damp;

	x += dx;
	y += dy;
	ang += dang;

	const double spaceWidth = 640. + 2. * 15.;
	const double spaceHeight = 400. + 2. * 15.;

	x = fmod(x + spaceWidth / 2., spaceWidth) - spaceWidth / 2.;
	if (x < 0.) x += spaceWidth;

	y = fmod(y + spaceHeight / 2., spaceHeight) - spaceHeight / 2.;
	if (y < 0.) y += spaceHeight;

	ang = fmod(ang, 2. * M_PI);
}
