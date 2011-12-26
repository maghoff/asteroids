#include <string>
#include <QDebug>
#include <QCoreApplication>
#include "game.hpp"

int main(int argc, char** argv) {
	QCoreApplication app(argc, argv);

	bool terminate_on_last_disconnect = false;

	for (char** arg = argv+1; arg != argv + argc; ++arg) {
		if ((*arg) == std::string("--terminate-on-last-disconnect")) {
			terminate_on_last_disconnect = true;
		}
	}

	srand(QTime::currentTime().msec());
	rand();

	Game game;

	if (terminate_on_last_disconnect) {
		QObject::connect(&game, SIGNAL(engineSuspended()), &app, SLOT(quit()));
	}

	qDebug() << "Ready...";

	return app.exec();
}
