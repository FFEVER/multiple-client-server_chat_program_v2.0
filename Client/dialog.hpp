#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include "privatedialog.hpp"

#include <QString>
#include <QStringList>
#include <QIntValidator>
#include <QRegularExpression>
#include <QMap>
#include <QTime>

#include <QTcpSocket>
#include <QHostAddress>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    void sendFileToAll(QString filePath,QString filename);
    void sendTextToServer(QString text);
    void onFileReceived(QString senderName,QString filename,QByteArray dataOfFile);

    ~Dialog();

public slots:
    void connected();
    void readyRead();
    void disconnected();
    void displayError(QAbstractSocket::SocketError socketError);
    void privateFinished(int result);
    void privateMessageSent(QString username,QString text);
    void privateFileSent(QString username,QString filePath,QString filename);


private slots:
    void on_pushButton_Login_clicked();

    void on_pushButton_Disconnect_clicked();

    void on_lineEdit_ChatInput_returnPressed();

    void on_pushButton_PrivateChat_clicked();

    void on_pushButton_ChooseFile_clicked();

private:
    Ui::Dialog *ui;
    quint16 port;
    QString username;
    QTcpSocket *tcpSocket;
    QString serverIP;
    QMap<QString,PrivateDialog*> privateChatList;
    quint32 blockSize = 0;
};

#endif // DIALOG_HPP
