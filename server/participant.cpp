#include <cmath>
#include "participant.hpp"
#include "bullet.hpp"
#include "spacemath.hpp"

Participant::Participant(Game *parent, QHostAddress host_, quint16 port_) :
	GameObject(parent),
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
const quint8 FIRE_BITMASK = 0x10;

const quint8 MSG_CONTROL_STATE = 0x02;

void Participant::incoming(const std::vector<char>& data) {
	lastSeen = QDateTime::currentDateTime();

	QByteArray ba(data.data(), data.size());
	QDataStream ds(&ba, QIODevice::ReadOnly);

	quint8 msgType;

	ds >> msgType;

	if (msgType == MSG_CONTROL_STATE) {
		/*quint32 time;*/
		quint8 controlBits;

		ds >> /*time >>*/ controlBits;

		bool up = controlBits & UP_BITMASK;
		bool down = controlBits & DOWN_BITMASK;
		bool left = controlBits & LEFT_BITMASK;
		bool right = controlBits & RIGHT_BITMASK;
		bool fire = controlBits & FIRE_BITMASK;

		dang = (left ? 1 : 0) + (right ? -1 : 0);
		dang *= 0.005;
		engine = up && !down;

		if (down) {
			dx = dy = 0;
		}

        if (fire) {
            double bdx = cos(ang) * 1;
            double bdy = sin(ang) * 1;
            game()->add(new Bullet(game(), x, y, bdx, bdy));
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

	const bool gravity = false;
	if (gravity) {
		double G = 0.000001;
		double gx = -G * x;
		double gy = -G * y;
		dx += gx;
		dy += gy;
	}

	const double damp = 0.999;
	dx *= damp;
	dy *= damp;

	x += dx;
	y += dy;
	ang += dang;

    cropToSpace(x, y);

	ang = fmod(ang, 2. * M_PI);
}

const quint8 OBJ_SHIP = 0x07;

void Participant::serializeStatus(QDataStream& ds) {
	//ds << size in bytes, for skipping?;
	ds << OBJ_SHIP;
	ds << color.r << color.g << color.b;
	ds << x << dx;
	ds << y << dy;
	ds << ang << dang;
	ds << (quint8)(engine ? 1 : 0);
}

Game *Participant::game() {
    return (Game*) parent();
}
