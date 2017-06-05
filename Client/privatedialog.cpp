#include "privatedialog.hpp"
#include "ui_privatedialog.h"

PrivateDialog::PrivateDialog(QString myUsername,QString username,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrivateDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Private Chat");
    this->username = username;
    this->myUsername = myUsername;
    ui->label_username->setText(this->username);

}

PrivateDialog::~PrivateDialog()
{
    delete ui;
}

void PrivateDialog::addNewMessage(QString time, QString message){
    ui->textEdit_chatDisplay->append(
                "<b>"+username+" ["+time+"] </b>: "+message);
}

void PrivateDialog::on_lineEdit_chatInput_returnPressed()
{
    QString text = ui->lineEdit_chatInput->text().trimmed();

    if(text.isEmpty())
        return;
    //get current time string
    QString currentTime = QTime::currentTime().toString("H:m A");

    ui->textEdit_chatDisplay->append("<font color=\"MediumBlue\"><b>"+myUsername+" ["+currentTime+"] :</b> "+text +"</font>");

    // emit the signal if you are not talking to your self
    if(this->myUsername != this->username){
        emit messageSent(username,text);
    }

    ui->lineEdit_chatInput->clear();
}

void PrivateDialog::on_pushButton_ChooseFile_clicked()
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
        //get current time string
        QString currentTime = QTime::currentTime().toString("H:mm A");
        //add to chat display
        ui->textEdit_chatDisplay->append("<font color=\"MediumBlue\"><b>"+myUsername+" ["+currentTime+"] send a file \""+filename+"\"</b></font>");
        emit fileSend(username,filePath,filename);
    }
}
