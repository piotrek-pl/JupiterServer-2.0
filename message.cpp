#include <QIODevice>
#include <QTextCodec>
#include "message.h"
#include <QDebug>


Message::Message(QByteArray source)
{
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> this->senderId >> this->receiverId >> this->content;
}

Message::Message(quint32 senderId, quint32 receiverId, QString content)
{
    this->senderId = senderId;
    this->receiverId = receiverId;
    this->content = content;
}
