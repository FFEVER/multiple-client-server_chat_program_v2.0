#include "dialog.hpp"
#include "ui_dialog.h"

//! PUBLIC
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("MultiThreading Chat Program v2.0::Server side");
    ui->stackedWidget->setCurrentWidget(ui->page_1); // set current page
    validator_port = new QIntValidator(1,65535,ui->lineedit_port); // set up int validator for port input

    //! CONNECT SIGNAL&SLOT
    connect(ui->stackedWidget,SIGNAL(currentChanged(int)),this->validator_port,SLOT(deleteLater())); // when page changed delete validator_int
    connect(&server,SIGNAL(connected(ClientThread*)),this,SLOT(on_clientThread_connected(ClientThread*))); // when client thread connected
    connect(&server,SIGNAL(clientDisconnected(ClientThread*)),this,SLOT(on_clientThread_disconnected(ClientThread*))); // when client thread disconnected
    connect(&server,SIGNAL(clientUsernameChanged(int,QString)),this,SLOT(on_clientThread_usernameChanged(int,QString))); // when client thread changed username
    //! CONNECT SIGNAL&SLOT


}

Dialog::~Dialog()
{
    server.close(); //close the listening port
    delete ui;
}

//! PUBLIC


//! SLOTS

void Dialog::on_pushButton_start_clicked()
{
    //Start listening on a specified port
    int pos = 0; // just for validate func.
    QString port = ui->lineedit_port->text( ) ; // input QString port.
    //Check if port is invalid or not 0 - 65535
    if(this->validator_port->validate(port,pos) == QIntValidator::Invalid
            || port.isEmpty()){
        QMessageBox::warning(this,tr("Input Error"),tr("Invalid port.\n(Must be 1 - 65535)"));
        return;
    }
    else{
        ui->stackedWidget->setCurrentWidget(ui->page_2); // change page.
        ui->lineEdit_Display_Port->setText(port);
        server.setPort(port.toUInt()); // convert port QString to quint and store it.
    }
    this->startServer();

}


void Dialog::on_clientThread_connected(ClientThread* clientThread){
    ui->textEdit_ServerLog->append(tr("%1 connected.").arg(clientThread->getSocketDescriptor()));
    ui->listWidget_OnlineUser->addItem(QString::number(clientThread->getSocketDescriptor())
                                       + " : " + clientThread->getUsername());

}

void Dialog::on_clientThread_disconnected(ClientThread* clientThread){
    //Append the action to server log screen.
    ui->textEdit_ServerLog->append(tr("%1 disconnected.").arg(clientThread->getSocketDescriptor()));
    //Remove user from online list
    QList<QListWidgetItem*> itemWidgets = ui->listWidget_OnlineUser->findItems(
                QString::number(clientThread->getSocketDescriptor())
                + " : " + clientThread->getUsername(),Qt::MatchExactly);
    foreach(QListWidgetItem* item,itemWidgets){
        delete item;
    }

}

void Dialog::on_clientThread_usernameChanged(int socketDescriptor,QString uname){
    QString pattern = QString::number(socketDescriptor) + " : ";

    QList<QListWidgetItem*> itemWidgets = ui->listWidget_OnlineUser->findItems(
                QString::number(socketDescriptor) + " : ",Qt::MatchContains);
    foreach(QListWidgetItem* item,itemWidgets){
        item->setText(pattern + uname);
    }

}

void Dialog::on_pushButton_Kick_clicked()
{

    if(!ui->listWidget_OnlineUser->count()){
        return;
    }
    if(ui->listWidget_OnlineUser->selectedItems().count() == 0){
        return;
    }

    //get text from current item on list online user
    QString text = ui->listWidget_OnlineUser->currentItem()->text();

    //set expression of string to derive socketdescriptor out
    QRegularExpression rex("(\\d+) : ");
    QRegularExpressionMatch match = rex.match(text);

    int socketDescriptor = 0;
    if(match.hasMatch()){
        socketDescriptor = match.captured(1).toInt();
        qDebug() << socketDescriptor;
    }

    server.kickFromServer(socketDescriptor);
//    emit pushButton_Kick_clicked(socketDescriptor);
}

void Dialog::on_pushButton_Quit_clicked()
{
    this->close();
}

//! SLOTS



//! PRIVATE
void Dialog::startServer(){
    /* SETUP SERVER */
    if(!server.StartServer()){
        QMessageBox::critical(this,tr("Error"),
                              tr("Unable to start the server: %1.").arg(server.errorString()));
        close();
        return;
    }

    /* GET LOCAL LISTENING ADDRESS */
    QString ipAddress;
    QList<QHostAddress> ipAddressList = QNetworkInterface::allAddresses();
    //use the first non-localhost IPv4 address.
    for(int i = 0; i < ipAddressList.size(); ++i){
        if(ipAddressList.at(i) != QHostAddress::LocalHost &&
           ipAddressList.at(i).toIPv4Address()){
            ipAddress = ipAddressList.at(i).toString();
            break;
        }
    }
    //if we did not find one, use IPv4 localhost
    if(ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    ui->lineEdit_IPAddr->setText(ipAddress);
    ui->textEdit_ServerLog->append("Listening for incoming connection...\n");


}


//! PRIVATE

