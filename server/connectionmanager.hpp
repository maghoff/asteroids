#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <QObject>
#include <QTcpServer>

class ConnectionManager : public QObject {
    Q_OBJECT

	QTcpServer s;

public:
    explicit ConnectionManager(QObject *parent = 0);

signals:

public slots:

};

#endif // CONNECTIONMANAGER_HPP
