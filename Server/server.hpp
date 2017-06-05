#ifndef SERVER_HPP
#define SERVER_HPP

#include <QTcpServer>
#include <QObject>
#include <QLinkedList>
#include <QHash>
#include <QTime>
#include "clientthread.hpp"


class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    bool StartServer();
    void setPort(quint16 port);
    void kickFromServer(int socketDescriptor);
    void close();
    void sendTextToAll(QString text,ClientThread* except = NULL);
    void sendTextToOne(QString text,ClientThread* target);
    void sendDataFileToAll(QString text,QByteArray dataOfFile,ClientThread* except = NULL);
    void sendDataFileToOne(QString text,QByteArray dataOfFile,ClientThread* target);


signals:
    void connected(ClientThread *clientThread);
    void clientDisconnected(ClientThread* clientThread);
    void clientUsernameChanged(int socketDescriptor,QString uname);

public slots:
    void on_client_connected(ClientThread* clientThread);
    void on_client_disconnected(ClientThread* clientThread);
    void on_client_usernameChanged(QString uname);
    void on_client_textSend(QString uname,QString text);
    void on_client_privateTextSend(QString uname,QString receiverName,QString text);
    void on_client_fileSend(QString uname, QString filename, QByteArray dataOfFile);
    void on_client_privateFileSend(QString uname,QString receiverName, QString filename, QByteArray dataOfFile);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
private:
    quint16 port = 7777; //default port
    void deleteClientFromList(ClientThread* client);
    QMap<ClientThread*,QString> users;
    QLinkedList<ClientThread*> clientThreadList;
};

#endif // SERVER_HPP
