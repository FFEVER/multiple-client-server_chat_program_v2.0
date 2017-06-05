#include "dialog.hpp"
#include "ui_dialog.h"


//! PUBLIC
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("MultiThreading Chat Program v2.0::Client side");
    ui->stackedWidget->setCurrentWidget(ui->Page_1);


    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket,SIGNAL(connected()),this,SLOT(connected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disconnected()));


}

Dialog::~Dialog()
{
    foreach(PrivateDialog* pmDialog,privateChatList.values()){
        pmDialog->close();
    }

    delete ui;
}

void Dialog::sendTextToServer(QString text){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = tcpSocket->write(block);
        x+=y;
        qDebug() << x;
    }
    qDebug() << "-----";

}

void Dialog::sendFileToAll(QString filePath,QString filename){
    //send file through fileTcpSocket

    //block to be sent
    QByteArray block;
    QFile file(filePath);
    //open file and check error
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"","Can't open file for transfer");
        return;
    }
    //datastream
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);
    QString command = "/fileAll:"+filename+"\n";
    //initial block size to 0 and input aspecific command to block
    out << (quint32)0 << command;
    //read all data of file to QByteArray
    QByteArray dataOfFile = file.readAll();
    //close the file
    file.close();
    //add it to block
    block.append(dataOfFile);
    //seek to the beginning
    out.device()->seek(0);
    //set the block size to actual block size - the size of stored block size
    out << (quint32)(block.size() - sizeof(quint32));

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = tcpSocket->write(block);
        x+=y;
        qDebug() << "sent file: "<< x << "of " << block.size();
    }
    qDebug() << "-----";
}

void Dialog::onFileReceived(QString senderName,QString filename,QByteArray dataOfFile){
    QMessageBox::StandardButtons reply;
    reply = QMessageBox::question(this,tr("File received"),
                             tr("%1 send you a file \"%2\"\nDo you want to save a file?")
                                  .arg(senderName,filename));
    if(reply == QMessageBox::No)
        return;
    QString homePath = QDir::homePath();
    QString filePath = QFileDialog::getExistingDirectory(this,
                                                    tr("Open Directory"),
                                                    homePath,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QFile outFile(filePath + "/" +filename);
    if(!outFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"","Error: Cannot open a file for writting.");
        return;
    }
    outFile.write(dataOfFile);

    outFile.close();

}

//! PUBLIC

//! PUBLIC SLOTS
void Dialog::connected(){
    ui->stackedWidget->setCurrentWidget(ui->Page_2);
    ui->lineEdit_ServerIP->setText(this->serverIP + "::" + QString::number(this->port));
    // And send our username to the chat server.
//    tcpSocket->write(QString("/username:" + username + "\n").toUtf8());
    //make the message string
    QString messageUsername = "/username:" + username +"\n";
    this->sendTextToServer(messageUsername);


}

void Dialog::readyRead(){
    QRegularExpression regex_text("^/text:(.*)/(.*) : (.*)\n$");
    QRegularExpression regex_server("^/server:(.*)\n$");
    QRegularExpression regex_users("^/users:(.*)\n$");
    QRegularExpression regex_private("^/pm:(.*)/(.*) : (.*)\n$");
    QRegularExpression regex_error("^/nameDup:(.*)\n$");
    QRegularExpression regex_fileAll("^/fileAll:(.*)/(.*) : (.*)\n$");
    QRegularExpression regex_filePrivate("^/filePrivate:(.*)/(.*) : (.*)\n$");
    QRegularExpressionMatch match;

    qDebug() << "1.canReadLine = "<< tcpSocket->canReadLine();
    qDebug() << "1.blockSize = " << blockSize;
    while(tcpSocket->canReadLine()){
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_8);

    if (blockSize == 0){
        if(tcpSocket->bytesAvailable() < sizeof(quint32))
            return;
        in >> blockSize;
        qDebug() << "blockSize "<< blockSize;
    }
    if(tcpSocket->bytesAvailable() < blockSize){
        qDebug() << "byteAval "<< tcpSocket->bytesAvailable();
        return;
    }

    QString data;
    in >> data;

    qDebug() << data;
    //match for text message
    match = regex_text.match(data);
    if(match.hasMatch()){
        //update chat display
        ui->textEdit_ChatDisplay->append(tr("<b>%1 <font color = \"LightGrey\">[%2]</font>: </b> %3")
                                         .arg(match.captured(2),match.captured(1),match.captured(3)));
    }
    //match for server message
    match = regex_server.match(data);
    if(match.hasMatch()){
        //update chat display
        ui->textEdit_ChatDisplay->append(
                    "<font color=\"gray\">" +match.captured(1)+"</font><br>");
    }
    //match for users list update
    match = regex_users.match(data);
    if(match.hasMatch()){
        //Add all users to online users list
        QStringList users = match.captured(1).split(",");
        ui->listWidget_OnlineUsers->clear();
        foreach(QString user,users){
            new QListWidgetItem(QPixmap("://images/User-icon.png"), user, ui->listWidget_OnlineUsers);
            if(user == this->username){
                QList<QListWidgetItem*> list = ui->listWidget_OnlineUsers->findItems(user,Qt::MatchExactly);
                foreach(QListWidgetItem* item,list){
                    item->setTextColor(Qt::darkRed);
                }
            }

        }
    }
    //match for private message
    match = regex_private.match(data);
    if(match.hasMatch()){
        QString senderName = match.captured(2);
        QString time = match.captured(1);
        QString text = match.captured(3);
        //if already open just append it
        if(!privateChatList.contains(senderName)){
            privateChatList[senderName] = new PrivateDialog(this->username,senderName,this);
            connect(privateChatList[senderName],SIGNAL(finished(int)),this,SLOT(privateFinished(int)));
            connect(privateChatList[senderName],SIGNAL(messageSent(QString,QString)),this,SLOT(privateMessageSent(QString,QString)));
            connect(privateChatList[senderName],SIGNAL(fileSend(QString,QString,QString)),this,SLOT(privateFileSent(QString,QString,QString)));
            privateChatList[senderName]->show();

        }
        privateChatList[senderName]->addNewMessage(time,text);
    }
    //match for server error nampDuplicate
    match = regex_error.match(data);
    if(match.hasMatch()){

        QString new_name = match.captured(1);
        QMessageBox::information(this,tr("Information"),"Your name \""+this->username+"\" is duplicated.\n"+
                                                        "Your name has changed to \""+new_name+"\".");
        this->username = new_name;
    }
    //match for file received
    match = regex_fileAll.match(data);
    if(match.hasMatch()){
        QString time = match.captured(1);
        QString senderName = match.captured(2);
        QString filename = match.captured(3);
        QByteArray dataOfFile = tcpSocket->readAll();
        ui->textEdit_ChatDisplay->append(tr("<b>%1 <font color = \"LightGrey\">[%2]</font> sent a file <font color = \"DarkSlateBlue\">\"%3\"</font></b>")
                                         .arg(senderName,time,filename));
        onFileReceived(senderName,filename,dataOfFile);
    }
    //match for private file received
    match = regex_filePrivate.match(data);
    if(match.hasMatch()){
        QString time = match.captured(1);
        QString senderName = match.captured(2);
        QString filename = match.captured(3);
        QByteArray dataOfFile = tcpSocket->readAll();
        //if already open just append it
        if(!privateChatList.contains(senderName)){
            privateChatList[senderName] = new PrivateDialog(this->username,senderName,this);
            connect(privateChatList[senderName],SIGNAL(finished(int)),this,SLOT(privateFinished(int)));
            connect(privateChatList[senderName],SIGNAL(messageSent(QString,QString)),this,SLOT(privateMessageSent(QString,QString)));
            connect(privateChatList[senderName],SIGNAL(fileSend(QString,QString,QString)),this,SLOT(privateFileSent(QString,QString,QString)));
            privateChatList[senderName]->show();
        }
        QString text = "sent a file <font color = \"DarkSlateGray\"><b>\"" + filename + "\"</b></font>";
        privateChatList[senderName]->addNewMessage(time,text);
        onFileReceived(senderName,filename,dataOfFile);
    }
    blockSize = 0;
    qDebug() << "2.canReadLine = "<< tcpSocket->canReadLine();

    }
}

void Dialog::disconnected(){
    ui->stackedWidget->setCurrentWidget(ui->Page_1);
    ui->textEdit_ChatDisplay->clear();
    ui->lineEdit_ServerIPAddr->setFocus(Qt::TabFocusReason);
    QMessageBox::information(this,tr("Disconnected"),tr("You are disconnected from server."));

}

void Dialog::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Socket Error"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Socket Error"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Socket Error"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }
}

void Dialog::privateFinished(int result){
    PrivateDialog *privateDialog = (PrivateDialog*)sender();
    privateChatList.remove(privateDialog->getUsername());
    qDebug() << "removed";
}

void Dialog::privateMessageSent(QString username,QString text){
//    tcpSocket->write(QString("/pm:"+username+ " : "+text+"\n").toUtf8());
    QString message = "/pm:"+username+" : "+text + "\n";
    this->sendTextToServer(message);

}

void Dialog::privateFileSent(QString username,QString filePath,QString filename){
    //block to be sent
    QByteArray block;
    QFile file(filePath);
    //open file and check error
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"","Can't open file for transfer");
        return;
    }
    //datastream
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);
    QString command = "/filePrivate:"+username+" : "+filename+"\n";
    //initial block size to 0 and input aspecific command to block
    out << (quint32)0 << command;
    //read all data of file to QByteArray
    QByteArray dataOfFile = file.readAll();
    //close the file
    file.close();
    //add it to block
    block.append(dataOfFile);
    //seek to the beginning
    out.device()->seek(0);
    //set the block size to actual block size - the size of stored block size
    out << (quint32)(block.size() - sizeof(quint32));

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = tcpSocket->write(block);
        x+=y;
        qDebug() << "sent file: "<< x << "of " << block.size();
    }
    qDebug() << "-----";
}

//! PUBLIC SLOTS

void Dialog::on_pushButton_Login_clicked()
{
    QString serverIP = ui->lineEdit_ServerIPAddr->text();
    QString port = ui->lineEdit_port->text();
    QString username = ui->lineEdit_username->text();
    QIntValidator portValidator(1,65535);
    int pos = 0;

    if(portValidator.validate(port,pos) == QIntValidator::Invalid || port.isEmpty()){
        QMessageBox::warning(this,tr("Input Error"),tr("Invalid port.\n(Must be 1 - 65535)"));
        return;
    }
    else if(username.isEmpty()){
        QMessageBox::warning(this,tr("Input Error"),tr("Invalid username.\n(Must contain some characters.)"));
        return;
    }

    QHostAddress servAddr(serverIP);
    this->serverIP = serverIP;
    this->port = port.toUInt();
    this->username = username;

    tcpSocket->connectToHost(servAddr,this->port);
    tcpSocket->waitForConnected(30000); //wait 30 seconds for socket to be connected to host successfully
                                        //if not connected, emit error.
}

void Dialog::on_pushButton_Disconnect_clicked()
{
    foreach(PrivateDialog *pmDialog,privateChatList.values()){
        pmDialog->close();
    }
    tcpSocket->close();
}

void Dialog::on_lineEdit_ChatInput_returnPressed()
{
    QString text = ui->lineEdit_ChatInput->text().trimmed();

    if(text.isEmpty())
        return;

    //add a specific function to text in order to be recognize by server that this is a text message
    //and send text over tcp socket.
//    tcpSocket->write(QString("/text:" + text +"\n").toUtf8());
    QString message = "/text:"+text +"\n";
    this->sendTextToServer(message);

    //get current time string
    QString currentTime = QTime::currentTime().toString("H:m A");
    //add to chat display
    ui->textEdit_ChatDisplay->append("<font color=\"DarkRed\"><b>"+username+" ["+currentTime+"]"+" :</b> " + text+"</font>");

    //clear chat input
    ui->lineEdit_ChatInput->clear();
}

void Dialog::on_pushButton_PrivateChat_clicked()
{
    if(!ui->listWidget_OnlineUsers->count()){
        return;
    }
    if(ui->listWidget_OnlineUsers->selectedItems().count() == 0){
        return;
    }

    //get text from current item on list online user
    QString username = ui->listWidget_OnlineUsers->currentItem()->text();
    //don't open new window if there is an already opened window
    if(privateChatList.contains(username)){
        return;
    }

    privateChatList[username] = new PrivateDialog(this->username,username,this);
    connect(privateChatList[username],SIGNAL(finished(int)),this,SLOT(privateFinished(int)));
    connect(privateChatList[username],SIGNAL(messageSent(QString,QString)),this,SLOT(privateMessageSent(QString,QString)));
    connect(privateChatList[username],SIGNAL(fileSend(QString,QString,QString)),this,SLOT(privateFileSent(QString,QString,QString)));
    privateChatList[username]->show();

}

void Dialog::on_pushButton_ChooseFile_clicked()
{
    //set home directory to user home path
    QString homePath = QDir::homePath();
    //open file dialog and choose a file and get the file path
    QString filePath = QFileDialog::getOpenFileName(this,tr("Open a file"),homePath,tr("All files (*.*)"),NULL,QFileDialog::DontResolveSymlinks);
    //get the filename
    QString filename = filePath.section("/",-1);
    //do nothing if no file selected
    if(filename.isEmpty())
        return;

    //confirmation
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"Send a file",tr("Do you want to send \"%1\"?").arg(filename));

    if (reply == QMessageBox::No )
        return;
    else{
        sendFileToAll(filePath,filename);
        //get current time string
        QString currentTime = QTime::currentTime().toString("H:mm A");
        //add to chat display
        ui->textEdit_ChatDisplay->append("<font color=\"DarkRed\"><b>"+username+" ["+currentTime+"] send a file \""+filename+"\"</b></font>");
    }


}
