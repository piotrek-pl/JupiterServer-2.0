#include "server.h"


Server::Server(QObject *parent)
    : QObject{parent}
{

}

Server::~Server()
{

}

bool Server::connectToDatabase()
{
    database = QSqlDatabase::addDatabase("QMYSQL");
    database.setHostName("77.237.31.25");
    database.setPort(3306);
    database.setDatabaseName("jupiter");
    database.setUserName("pi");
    database.setPassword("raspberrypi");

    return database.open();
}

void Server::startServer()
{
    chatServer = new QTcpServer();
    chatServer->setMaxPendingConnections(10);
    connect(chatServer, &QTcpServer::newConnection, this, &Server::newClientConnection);


    if (connectToDatabase())
    {
        qDebug() << "Connected to database!";

        chatServer = new QTcpServer();
        chatServer->setMaxPendingConnections(10);
        connect(chatServer, &QTcpServer::newConnection, this, &Server::newClientConnection);

        if (chatServer->listen(QHostAddress::Any, 1234))
        {
            qDebug() << "Server has started. Listening to port 1234.";
        }
        else
        {
            qDebug() << "Server failed to start. Error: " + chatServer->errorString();
        }
    }
    else
    {
        qDebug() << "Failed to connect to database.";
    }

}

void Server::newClientConnection()
{
    QTcpSocket *client = chatServer->nextPendingConnection();
    QString ipAddress = client->peerAddress().toString();
    int port = client->peerPort();

    connect(client, &QTcpSocket::disconnected, this, &Server::socketDisconnected);
    //connect(client, &QTcpSocket::connected, this, &Server::socketConnected);
    connect(client, &QTcpSocket::readyRead, this, &Server::socketReadyRead);
    connect(client, &QTcpSocket::stateChanged, this, &Server::socketStateChanged);

    qDebug() << "Socket connected from " + ipAddress.mid(IPV4) + ":" + QString::number(port);
}

void Server::socketDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    int port = client->peerPort();
    qDebug() << "Socket disconnected from " + socketIpAddress.mid(IPV4) + ":" + QString::number(port);

    qDebug() << "senderId =" << socketAndIdMap[client];
    setUnavailableUserInUsersTable(socketAndIdMap[client]);
    socketAndIdMap.remove(client);
}

/*void Server::socketConnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    int port = client->peerPort();
    qDebug() << "Socket connected from " + socketIpAddress.mid(IPV4) + ":" + QString::number(port);
}*/

void Server::setUnavailableUserInUsersTable(quint32 senderId)
{
    QString sqlCommand = "UPDATE users SET available = 0 WHERE id = " + QString::number(senderId);
    QSqlQuery query(database);
    if (query.exec(sqlCommand))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "Updated available state for user" << senderId;
        }
        else
        {
            qDebug() << "Update available state for user" << senderId << "failed.";
        }
    }
}

void Server::socketReadyRead()
{
    qDebug() << "reading...";
    QTcpSocket *client = qobject_cast<QTcpSocket*>(QObject::sender());
    if (!client) return;
    QDataStream tcpStream(client);
    processTcpStream(tcpStream);
}

void Server::processTcpStream(QDataStream& tcpStream)
{
    qDebug() << "Jestem w metodzie processTcpStream(QDataStream& tcpStream)";
    while (true)
    {
        tcpStream.startTransaction();
        quint32 senderId, receiverId;
        QString content;
        tcpStream >> senderId >> receiverId >> content;
        Message message(senderId, receiverId, content);
        if (!tcpStream.commitTransaction())
        {
                break;
        }
        processMessage(message);
    }
}

void Server::processMessage(Message message)
{
    qDebug() << "Jestem w metodzie processMessage(Message message)";
    if (message.getReceiverId() == NO_ID)
    {
        qDebug() << "\tFirst message. User id:" << message.getSenderId();
        socketAndIdMap.insert(qobject_cast<QTcpSocket*>(QObject::sender()), message.getSenderId());
        setAvailableUserInUsersTable(message.getSenderId());
    }
    else
    {
        writeMessageToDatabase(message);
        qDebug() << message.getSenderId() << message.getReceiverId() << message.getContent();
        setNewMessageState(message.getSenderId(), message.getReceiverId());
    }
}

void Server::setNewMessageState(quint32 senderId, quint32 receiverId)
{
    qDebug() << "Jestem w setNewMessageState(quint32 senderId, quint32 receiverId)";

    QString updateNewMessageStateSqlCommand = QString("UPDATE %1_friends SET %1_friends.is_new_message = 1 "
                                              "WHERE %1_friends.id = %2")
                                              .arg(receiverId).arg(senderId);
    qDebug() << "\t" << updateNewMessageStateSqlCommand;
    QSqlQuery query(database);
    if (query.exec(updateNewMessageStateSqlCommand))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "\tUpdated is_new_message state";
        }
        else
        {
            qDebug() << "\tUpdate is_new_message state for user failed.";
        }
    }


    /*qDebug() << "jestem w setNew...";
    QString idToUsernameQuery = "SELECT username FROM users WHERE id = " + QString::number(receiverId);
    qDebug() << idToUsernameQuery;
    QSqlQuery query(database);
    if (query.exec(idToUsernameQuery))
    {
        qDebug() << "pierwszy if...";
        if (query.size() > 0)
        {
            query.next();
            QString username = query.value("username").toString();
            qDebug() << "Username to: " << username;
            QString updateNewMessageStateSqlCommand = "UPDATE " + username + "_friends SET " +
                    username + "_friends.is_new_message = 1 " +  + "WHERE " +
                    username + "_friends.id = " + "'" + QString::number(senderId) + "'";

            qDebug() << "komenda to: " << updateNewMessageStateSqlCommand;
            if (query.exec(updateNewMessageStateSqlCommand))
            {
                if (query.numRowsAffected() == 1)
                {
                    qDebug() << "Updated is_new_message state";
                }
                else
                {
                    qDebug() << "Update is_new_message state for user failed.";
                }
            }
        }
    }*/
}

void Server::writeMessageToDatabase(Message message)
{
    qDebug() << "Jestem w metodzie writeMessageToDatabase(Message message)";
    /*QString senderTableSqlCommand = "INSERT INTO " + QString::number(message.getSenderId()) + "_chat_" + QString::number(message.getReceiverId())
                        + " (message) " + "VALUES " + "('" + message.getContent() + "')";*/
    QString senderTableSqlCommand = "INSERT INTO " + QString::number(message.getSenderId()) + "_chat_" + QString::number(message.getReceiverId())
                        + " (sender_id, message) " + "VALUES " + "('" + QString::number(message.getSenderId()) + "', '" + message.getContent() + "')";
    /*QString receiverTableSqlCommand = "INSERT INTO " + QString::number(message.getReceiverId()) + "_chat_" + QString::number(message.getSenderId())
                        + " (message) " + "VALUES " + "('" + message.getContent() + "')";*/
    QString receiverTableSqlCommand = "INSERT INTO " + QString::number(message.getReceiverId()) + "_chat_" + QString::number(message.getSenderId())
                        + " (sender_id, message) " + "VALUES " + "('" + QString::number(message.getSenderId()) + "', '" + message.getContent() + "')";
    writeMessageToSpecificTable(senderTableSqlCommand);
    writeMessageToSpecificTable(receiverTableSqlCommand);
}

void Server::writeMessageToSpecificTable(QString sqlCommand)
{
    QSqlQuery query(database);
    qDebug() << "writeMessage...";
    if (query.exec(sqlCommand))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "write message to database completed";
        }
        else
        {
            qDebug() << "cannot write message to database";
        }
    }
}

void Server::setAvailableUserInUsersTable(quint32 senderId)
{
    QString sqlCommand = "UPDATE users SET available = 1 WHERE id = " + QString::number(senderId);
    QSqlQuery query(database);
    if (query.exec(sqlCommand))
    {
        if (query.numRowsAffected() == 1)
        {
            qDebug() << "Updated available state for user" << senderId;
        }
        else
        {
            qDebug() << "Update available state for user" << senderId << "failed.";
        }
    }
}

void Server::socketStateChanged(QAbstractSocket::SocketState state)
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    int port = client->peerPort();

    QString desc;

    if (state == QAbstractSocket::UnconnectedState)
        desc = "The socket is not connected.";
    else if (state == QAbstractSocket::HostLookupState)
        desc = "The socket is performing a host name lookup.";
    else if (state == QAbstractSocket::ConnectingState)
        desc = "The socket has started establishing a connection.";
    else if (state == QAbstractSocket::ConnectedState)
        desc = "A connection is established.";
    else if (state == QAbstractSocket::BoundState)
        desc = "The socket is bound to an address and port.";
    else if (state == QAbstractSocket::ClosingState)
        desc = "The socket is about to close (data may still be waiting to be written).";
    else if (state == QAbstractSocket::ListeningState)
        desc = "For internal use only.";

    qDebug() << "Socket state changed (" + socketIpAddress.mid(IPV4) + ":" + QString::number(port) + "): " + desc;
}
