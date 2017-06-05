#ifndef PRIVATEDIALOG_HPP
#define PRIVATEDIALOG_HPP

#include <QDialog>
#include <QString>
#include <QTime>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

namespace Ui {
class PrivateDialog;
}

class PrivateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrivateDialog(QString myUsername,QString username,QWidget *parent = 0);
    QString getUsername() const{return username;}
    void addNewMessage(QString time,QString message);
    ~PrivateDialog();

signals:
    void messageSent(QString username,QString message); //when user press enter in chat input
    void fileSend(QString username,QString filePath,QString filename);


private slots:
    void on_lineEdit_chatInput_returnPressed();

    void on_pushButton_ChooseFile_clicked();

private:
    Ui::PrivateDialog *ui;
    QString username;
    QString myUsername;
};

#endif // PRIVATEDIALOG_HPP
