#ifndef MESSAGE_H
#define MESSAGE_H
#include <QObject>
#include <QDataStream>


class Message
{
public:
    Message() {};
    Message(quint32 senderId, quint32 receiverId, QString content);
    Message(QByteArray source);
    quint32 getSenderId() { return senderId; }
    quint32 getReceiverId() { return receiverId; }
    QString getContent() { return content; }
    void setSenderId(quint32 senderId) { this->senderId = senderId; }
    void setReceiverId(quint32 receiverId) { this->receiverId = receiverId; }
    void setContent(QString content) { this->content = content; }
private:
    quint32 senderId;
    quint32 receiverId;
    QString content;
};

#endif // MESSAGE_H
