#ifndef CLIENTTHREAD_HPP
#define CLIENTTHREAD_HPP

#include <QObject>
#include <QThread>
#include <QTcpSocket>

#include <QDataStream>
#include <QString>
#include <QRegularExpression>

class ClientThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientThread(int SocketDescriptor,QTcpSocket *socket,QObject *parent = 0);
    QString getUsername() const{return username;}
    int getSocketDescriptor() const{return socketDescriptor;}
    QTcpSocket* getTcpSocket() const{return tcpSocket;}
    void setUsername(QString uname) {username = uname;}
    void close();
    friend bool operator==(ClientThread client1,ClientThread client2);

    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);
    void connected(ClientThread* clientThread);
    void clientDisconnected(ClientThread* clientThread);
    void usernameChanged(QString uname);
    void textSend(QString uname,QString text);
    void privateTextSend(QString uname,QString receiverName,QString text);
    void fileSend(QString uname,QString filename,QByteArray dataOfFile);
    void privateFileSend(QString uname,QString receiverName,QString filename,QByteArray dataOfFile);


public slots:
    void readyRead();
    void disconnected();

private:
    int socketDescriptor;
    QString username = "Annonymous";
    QTcpSocket *tcpSocket;
    quint32 blockSize = 0;
};

#endif // CLIENTTHREAD_HPP
