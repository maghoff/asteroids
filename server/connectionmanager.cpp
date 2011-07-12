#include "connectionmanager.hpp"

ConnectionManager::ConnectionManager(QObject *parent) :
    QObject(parent)
{
	s.listen(QHostAddress::Any, 50005);
}
