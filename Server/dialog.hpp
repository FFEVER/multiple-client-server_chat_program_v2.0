#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <QDialog>
#include <QMessageBox>

#include <QList>

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkInterface>

#include <QIntValidator>
#include <QRegularExpression>

#include "server.hpp"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
signals:
    void pushButton_Kick_clicked(int socketDescriptor);
public slots:

    void on_clientThread_connected(ClientThread* clientThread);
    void on_clientThread_disconnected(ClientThread* clientThread);
    void on_clientThread_usernameChanged(int socketDescriptor,QString uname);

private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_Kick_clicked();

    void on_pushButton_Quit_clicked();

private:
    Ui::Dialog *ui;
    QIntValidator *validator_port; //input port validator
    Server server;
    void startServer();

};

#endif // DIALOG_HPP


