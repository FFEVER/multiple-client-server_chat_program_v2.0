#include "clientthread.hpp"

ClientThread::ClientThread(int SocketDescriptor,QTcpSocket *socket,QObject *parent):
    QThread(parent)
{
    socketDescriptor = SocketDescriptor;
    this->tcpSocket = socket;
}

void ClientThread::close(){
    //Close socket
    this->tcpSocket->close();
    exit(0);
}

void ClientThread::run(){
    //Check if able to initialzies socketDescriptor
    if(!tcpSocket->setSocketDescriptor(this->socketDescriptor)){
        emit error(tcpSocket->error());
        return;
    }

    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(readyRead()),Qt::DirectConnection);
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disconnected()),Qt::DirectConnection);

    emit connected(this);

//    qDebug() << socketDescriptor << " Client Connected";

    exec();
}

void ClientThread::readyRead(){
    //set QRegExp to be match exactly with the specific pattern
    QRegularExpression regex_username("^/username:(.*)\n$");
    QRegularExpression regex_text("^/text:(.*)\n$");
    QRegularExpression regex_private("^/pm:(.*) : (.*)\n$");
    QRegularExpression regex_fileAll("^/fileAll:(.*)\n$");
    QRegularExpression regex_filePrivate("^/filePrivate:(.*) : (.*)\n$");
    QRegularExpressionMatch match;


    while(tcpSocket->canReadLine()){
        QDataStream in(tcpSocket);
        in.setVersion(QDataStream::Qt_5_8);
        qDebug() << "blockSize = "<< blockSize;
        if (blockSize == 0){
            if(tcpSocket->bytesAvailable() < sizeof(quint32))
                return;
            in >> blockSize;
            qDebug() << "!blockSize = "<< blockSize;
        }
        if(tcpSocket->bytesAvailable() < blockSize){
            qDebug() << "byteAvailable = " << tcpSocket->bytesAvailable();
            return;
        }

        QString data;
        in >> data;

        qDebug() << data;
        match = regex_username.match(data);
        if(match.hasMatch()){
            this->username = match.captured(1);
            emit usernameChanged(this->username);
        }
        match = regex_text.match(data);
        if(match.hasMatch()){
            QString text = match.captured(1);
            emit textSend(this->username,text);
        }
        match = regex_private.match(data);
        if(match.hasMatch()){
            QString receiverName = match.captured(1);
            QString text = match.captured(2);
            emit privateTextSend(this->username,receiverName,text);
        }
        match = regex_fileAll.match(data);
        if(match.hasMatch()){
            QString filename = match.captured(1);
            QByteArray dataOfFile = tcpSocket->readAll();
            emit fileSend(this->username,filename,dataOfFile);
        }
        match = regex_filePrivate.match(data);
        if(match.hasMatch()){
            QString target = match.captured(1);
            QString filename = match.captured(2);
            QByteArray dataOfFile = tcpSocket->readAll();
            emit privateFileSend(this->username,target,filename,dataOfFile);
        }
        blockSize = 0;
    }

}

void ClientThread::disconnected(){
    qDebug() << socketDescriptor << " Disconnected";
    emit clientDisconnected(this);

    tcpSocket->deleteLater();
    exit(0);
}


//! OPERATOR OVERLOADING
bool operator==(ClientThread client1,ClientThread client2){
    return client1.socketDescriptor == client2.socketDescriptor;
}

//! OPERATOR OVERLOADING
