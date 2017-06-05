#include "server.hpp"

Server::Server(QObject *parent):
    QTcpServer(parent)
{

}

//! PUBLIC
bool Server::StartServer(){
    if(!this->listen(QHostAddress::Any,port)){
        qDebug() << "Could not start server";
        return false;
    }
    else{
        qDebug() << "Listening...";
        return true;
    }
}

void Server::setPort(quint16 port){
    this->port = port;
    return;
}

void Server::close(){
    //close every client thread
    foreach(ClientThread* client,clientThreadList){
        client->close();
    }
    //close itself
    QTcpServer::close();
}

void Server::sendTextToAll(QString text,ClientThread* except){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();

    qint64 x = 0;
    foreach(ClientThread* eachClient,clientThreadList){
        if(except != NULL && eachClient == except)
            continue;
        x = 0;
        while(x < block.size()){
            qint64 y = eachClient->getTcpSocket()->write(block);
            x+=y;
            qDebug() << eachClient->getUsername()<< "/sent" << x ;
        }
        qDebug() << "-----";
    }
}


void Server::sendTextToOne(QString text,ClientThread* target){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = target->getTcpSocket()->write(block);
        x+=y;
        qDebug() << target->getUsername()<< " /sent " << x ;
        qDebug() << "-----";
    }
}

void Server::sendDataFileToAll(QString text,QByteArray dataOfFile,ClientThread* except){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;
    block.append(dataOfFile);

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();

    qint64 x = 0;
    foreach(ClientThread* eachClient,clientThreadList){
        if(except != NULL && eachClient == except)
            continue;
        x = 0;
        while(x < block.size()){
            qint64 y = eachClient->getTcpSocket()->write(block);
            x+=y;
            qDebug() << eachClient->getUsername()<< "/sent file: " << x ;
        }
        qDebug() << "-----";
    }
}

void Server::sendDataFileToOne(QString text,QByteArray dataOfFile,ClientThread* target){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;
    block.append(dataOfFile);

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = target->getTcpSocket()->write(block);
        x+=y;
        qDebug() << target->getUsername()<< "/sent file: " << x ;
    }
    qDebug() << "-----";
}

void Server::kickFromServer(int socketDescriptor){
    QLinkedList<ClientThread*>::iterator iter;
    //Find for the matching socketDescriptor in client list
    //and make them disconnect
    for(iter = clientThreadList.begin();iter != clientThreadList.end(); ++iter){
        if((*iter)->getSocketDescriptor() == socketDescriptor){
            (*iter)->close();
            return;
        }
    }

}

//! PUBLIC


//! SLOTS
void Server::on_client_connected(ClientThread* clientThread){
    clientThreadList.push_back(clientThread);
//    qDebug() << "added";


}

void Server::on_client_disconnected(ClientThread* clientThread){
    deleteClientFromList(clientThread);

    QStringList usernameList;
    foreach(QString username,users.values()){
        usernameList.append(username);
    }
    //Send message to every users that a user has left
    //and resend the online user list
//    foreach(ClientThread* eachClient,clientThreadList){
//        eachClient->getTcpSocket()->write(QString("/server:" + clientThread->getUsername() + " has left.\n").toUtf8());
//        eachClient->getTcpSocket()->write(QString("/users:" + usernameList.join(",") +"\n").toUtf8());
//    }
    QString messageUserLeft = "/server:" + clientThread->getUsername() + " has left.\n";
    QString messageUserNewList= "/users:" + usernameList.join(",") +"\n";
    sendTextToAll(messageUserLeft);
    sendTextToAll(messageUserNewList);

}

void Server::on_client_usernameChanged(QString uname){
    ClientThread* client = (ClientThread*)sender();
    QStringList nameList = users.values();
    //if name is duplicate add "_"+count to its name
    //and tell the client that their name has changed
    if(nameList.contains(uname)){
        int count = 1;
        //check for every other duplicated names
        QRegularExpression dupName("^"+uname+"_\\d+$");
        QRegularExpressionMatch matchName;
        foreach(QString name,nameList){
            if(dupName.match(name).hasMatch()){
                count++;
            }
        }

        uname = uname + "_" + QString::number(count);
//        client->getTcpSocket()->write(QString("/nameDup:"+uname+"\n").toUtf8());
        QString messageNameDup = "/nameDup:" + uname + "\n";
        sendTextToOne(messageNameDup,client);

        client->setUsername(uname);
    }
    users[client] = uname;

    QStringList usernameList;
    foreach(QString username,users.values()){
        usernameList.append(username);
    }

//    //Send message to every users that a user has join
//    //and resend the online user list
//    foreach(ClientThread* eachClient,clientThreadList){
//        eachClient->getTcpSocket()->write(QString("/server:" + uname + " has joined.\n").toUtf8());
//        eachClient->getTcpSocket()->write(QString("/users:" + usernameList.join(",") +"\n").toUtf8());
//    }


    //make the message string
    QString messageUserJoin = "/server:" + uname + " has joined.\n";
    QString messageUserNewList = "/users:" + usernameList.join(",")+"\n";
    sendTextToAll(messageUserJoin);
    sendTextToAll(messageUserNewList);

    emit clientUsernameChanged(client->getSocketDescriptor(),uname);
}

void Server::on_client_textSend(QString uname,QString text){
    //client that send the signal to this slot
    ClientThread* sender_client = (ClientThread*)sender();
    QString currentTime = QTime::currentTime().toString("H:m A/");

    QString message = "/text:"+currentTime + uname + " : " + text + "\n";
//    foreach(ClientThread* client,clientThreadList){
//        if(sender_client != client){
//            client->getTcpSocket()->write(message.toUtf8());
//        }
//    }
    sendTextToAll(message,sender_client);

}

void Server::on_client_privateTextSend(QString uname,QString receiverName,QString text){
    QString currentTime = QTime::currentTime().toString("H:mm A/");
    QString messagePm = "/pm:"+currentTime+uname+" : "+text+"\n";
    foreach(ClientThread* client,clientThreadList){
        if(client->getUsername() == receiverName){
            sendTextToOne(messagePm,client);
            return;
        }
    }
}


void Server::on_client_fileSend(QString uname,QString filename,QByteArray dataOfFile){
    ClientThread* client = (ClientThread*)sender();
    QString currentTime = QTime::currentTime().toString("H:mm A/");
    QString command = "/fileAll:"+currentTime + uname + " : " + filename + "\n";
    this->sendDataFileToAll(command,dataOfFile,client);
}

void Server::on_client_privateFileSend(QString uname,QString receiverName, QString filename, QByteArray dataOfFile){
    QString currentTime = QTime::currentTime().toString("H:mm A/");
    QString command = "/filePrivate:"+currentTime + uname + " : " + filename + "\n";
    foreach(ClientThread* client,clientThreadList){
        if(client->getUsername() == receiverName){
            sendDataFileToOne(command,dataOfFile,client);
            return;
        }
    }
}

//! SLOTS


//! PROTECTED
void Server::incomingConnection(qintptr socketDescriptor){
    qDebug() << socketDescriptor << " Connecting...";
    QTcpSocket *socket = new QTcpSocket();
    ClientThread *cliThread = new ClientThread(socketDescriptor,socket,this);


    connect(cliThread, SIGNAL(finished()),cliThread,SLOT(deleteLater()));
    connect(cliThread, SIGNAL(connected(ClientThread*)),this,SIGNAL(connected(ClientThread*)));
    connect(cliThread, SIGNAL(connected(ClientThread*)),this,SLOT(on_client_connected(ClientThread*)));
    connect(cliThread, SIGNAL(clientDisconnected(ClientThread*)),this,SIGNAL(clientDisconnected(ClientThread*)));
    connect(cliThread, SIGNAL(clientDisconnected(ClientThread*)),this,SLOT(on_client_disconnected(ClientThread*)));
    connect(cliThread, SIGNAL(usernameChanged(QString)),this,SLOT(on_client_usernameChanged(QString)));
    connect(cliThread, SIGNAL(textSend(QString,QString)),this,SLOT(on_client_textSend(QString,QString)));
    connect(cliThread, SIGNAL(privateTextSend(QString,QString,QString)),this,SLOT(on_client_privateTextSend(QString,QString,QString)));
    connect(cliThread, SIGNAL(fileSend(QString,QString,QByteArray)),this,SLOT(on_client_fileSend(QString,QString,QByteArray)));
    connect(cliThread, SIGNAL(privateFileSend(QString,QString,QString,QByteArray)),this,SLOT(on_client_privateFileSend(QString,QString,QString,QByteArray)));


    socket->moveToThread(cliThread);
    cliThread->start(); //Start thread
}

//! PROTECTED


//! PRIVATE
void Server::deleteClientFromList(ClientThread* client){
    // Called when on_client_disconnected slot is called.
    clientThreadList.removeOne(client);
    users.remove(client);

    qDebug() << "removed.";
}

//! PRIVATE
