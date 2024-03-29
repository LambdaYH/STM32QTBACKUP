#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
protected:
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:

    void on_btnOpen_clicked();

    void on_btnSend_clicked();

    void Read_Data();

    void on_btnClear_clicked();

private:
    void setYSVisable();
    void setJSVisable();
    bool check_frame(QString frame_str);
    void process_frame(QString frame_str);
    Ui::Dialog *ui;
    QSerialPort *serial;
};

#endif // DIALOG_H
