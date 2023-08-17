#include "databaseconnectionmanager.h"
#include "server.h"
#include <QDebug>
#include <QSqlQuery>
#include <QThread>

DatabaseConnectionManager::DatabaseConnectionManager(QObject *parent)
    : QObject{parent}
{

}

bool DatabaseConnectionManager::reconnectDatabase(QSqlDatabase database)
{
    qDebug() << "DatabaseConnectionManager::reconnectDatabase(QSqlDatabase database)";
    while (!checkConnection(database))
    {
        qDebug() << "\tPołączenie utracone";
        Server::connectToDatabase();
        QThread::msleep(3000);
    }
    qDebug() << "Przywrócono połączenie";
    emit databaseConnectionRestored();
    return true;
}

bool DatabaseConnectionManager::checkConnection(QSqlDatabase database)
{
    //qDebug() << "DatabaseConnectionManager::checkConnection(QSqlDatabase database)";
    QSqlQuery query("SELECT 1", database);
    if (query.exec() && query.next() && query.value(0).toInt() == 1)
    {

        return true;
    }
    else
    {
        qDebug() << "Błąd podczas sprawdzania połączenia.";
        emit databaseConnectionLost();
        return false;
    }
}

