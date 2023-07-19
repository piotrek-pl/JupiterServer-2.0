#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QList>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "user.h"
#include "message.h"

class Server : public QObject
{
    Q_OBJECT
private:
    QTcpServer *chatServer;
    QSqlDatabase database;
    QList<User*> connectedUsers;
    //QList<QTcpSocket*> *allClients;
    QMap<QTcpSocket*, quint32> socketAndIdMap;


    enum ipAddressFormat { IPV4 = 7 };
    enum ID { NO_ID = 0 };

    bool connectToDatabase();
    QSqlDatabase getDatabase() { return database; }
    //void sendMessageToClients(QString message);
    void processTcpStream(QDataStream& tcpstream);
    void writeMessageToDatabase(Message message);
    void writeMessageToSpecificTable(QString SqlCommand);
    void writeMessageToReceiverDb(QString sqlCommand);
    void processMessage(Message message);
    void setAvailableUserInUsersTable(quint32 senderId);
    void setUnavailableUserInUsersTable(quint32 senderId);
    void setNewMessageState(quint32 senderId, quint32 receiverId);

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();
    void startServer();

private slots:
    void newClientConnection();
    void socketDisconnected();
    //void socketConnected();
    void socketReadyRead();
    void socketStateChanged(QAbstractSocket::SocketState state);
};

#endif // SERVER_H
