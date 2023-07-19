#ifndef USER_H
#define USER_H

#include <QHostAddress>
#include <QObject>
#include <QTcpSocket>

class User : public QObject
{
    Q_OBJECT
public:
    explicit User(QObject *parent = nullptr);
    void setSocket(QTcpSocket* socket) { this->socket = socket; }
    void setId(quint32 id) { this->id = id; }

signals:

private:
    quint32 id;
    QString username;
    QTcpSocket *socket;

};

#endif // USER_H
