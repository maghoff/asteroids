#include <QDebug>
#include <QCoreApplication>
#include "game.hpp"

int main(int argc, char** argv) {
	QCoreApplication app(argc, argv);

	srand(QTime::currentTime().msec());
	rand();

	Game game;

	qDebug() << "Ready...";

	return app.exec();
}
