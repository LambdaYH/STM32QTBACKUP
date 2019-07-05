#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    //查找可用的串口
    ui->comboBox->clear();
    ui->LED1->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->LED2->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->LED3->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->LED4->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox->addItem(serial.portName());
            serial.close();
        }
    }
    ui->comboBox_2->setCurrentIndex(4);
    ui->btnSend->setEnabled(false);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_btnOpen_clicked()
{
    if(ui->btnOpen->text() == "打开串口")
    {
        serial = new QSerialPort;
        //设置串口名
        serial->setPortName(ui->comboBox->currentText());
        //打开串口
        serial->open(QIODevice::ReadWrite);
        //设置波特率
        int bps = ui->comboBox_2->currentText().toInt();
        serial->setBaudRate(bps);
        //设置数据位数
        serial->setDataBits(QSerialPort::Data8);
        //设置奇偶校验
        serial->setParity(QSerialPort::NoParity);
        //设置停止位
        serial->setStopBits(QSerialPort::OneStop);
        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl);
        //关闭设置菜单使能
        ui->comboBox->setEnabled(false);
        ui->comboBox_2->setEnabled(false);
        ui->btnOpen->setText("关闭串口");
        ui->btnSend->setEnabled(true);
        //连接信号槽
        QObject::connect(serial, &QSerialPort::readyRead, this, &Dialog::Read_Data);
    }
    else
    {
        //关闭串口
        serial->clear();
        serial->close();
        serial->deleteLater();
        //恢复设置使能
        ui->comboBox->setEnabled(true);
        ui->comboBox_2->setEnabled(true);
        ui->btnOpen->setText("打开串口");
        ui->btnSend->setEnabled(false);
    }
}

void Dialog::on_btnSend_clicked()
{
    QString str = ui->textSend->text() + "\n";
    QByteArray buf = str.toLatin1();
    serial->write(buf);
}

void Dialog::Read_Data()
{
    static QString frame_buff;
    QByteArray buf;
    buf = serial->readAll();
    if(!buf.isEmpty())
    {
//       QString str = ui->textRecv->toPlainText();
//       str += QString(buf);
//       ui->textRecv->clear();
//       ui->textRecv->append(str);
        frame_buff +=QString(buf);
        int len = frame_buff.indexOf("\n");
        while(len>=0){
//            ui->textRecv->append(frame_buff.left(len));
            process_frame(frame_buff.left(len));
            frame_buff = frame_buff.mid(len + 1);
            len = frame_buff.indexOf("\n");
        }
    }
    buf.clear();
}

void Dialog::on_btnClear_clicked()
{
    ui->textRecv->clear();
    ui->textSend->clear();
}

bool Dialog::check_frame(QString frame_str)
{
    QByteArray barry = frame_str.toLatin1();
    int len = barry.count();
    if(len>2)
    {
        bool bok;
        int chksum = frame_str.right(2).toInt(&bok,16);
        int sum =0;
        for (int i=0;i<len-2;++i)
            sum+=barry[i];
        sum %=256;
        if (sum == chksum)
                return true;
    }
    return false;
}

void Dialog::process_frame(QString frame_str)
{
    frame_str = frame_str.trimmed();
    if(!check_frame(frame_str))
    {
        ui->textRecv->append(frame_str+"check sum error!\n");
        return;
    }

    frame_str=frame_str.left(frame_str.length()-2);

    int i,j,k;
    int led_sta[4],btn_sta[4];
    int adval = 0,ax=0,ay=0,az=0,gx=0,gy=0,gz=0;
    double fvol =0,fpitch=0,froll=0,fyaw=0;
    QString tstr;

    if(frame_str.indexOf("7MBD")==0 && frame_str.length()>=12){
        for(i=0;i<4;i++){
            led_sta[i]=(frame_str[4+i]=='0')?0:1;
        }
        for(i=0;i<4;i++){
            btn_sta[i] = (frame_str[8+i]=='U')?0:1;
        }
        tstr.sprintf("%s%s%s%s%s%s%s%s\n",
                     led_sta[0]?"💡":"⚪",led_sta[1]?"💡":"⚪",led_sta[2]?"💡":"⚪",led_sta[3]?"💡":"⚪",
                     btn_sta[0]?"▁":"█",btn_sta[1]?"▁":"█",btn_sta[2]?"▁":"█",btn_sta[3]?"▁":"█");
                     ui->LED1->setText(led_sta[0]?"💡":"⚪");
                     ui->LED2->setText(led_sta[1]?"💡":"⚪");
                     ui->LED3->setText(led_sta[2]?"💡":"⚪");
                     ui->LED4->setText(led_sta[3]?"💡":"⚪");
    }
    else if(frame_str.indexOf("7MY,")==0 && frame_str.length()>5){
        i=5;
        j=frame_str.indexOf(',',i);
        k=0;
        while(j>0&&k<6){
            switch(k){
            case 0:adval = frame_str.mid(i,j-i).toInt();break;
            case 1:ax = frame_str.mid(i,j-i).toInt();break;
            case 2:ay = frame_str.mid(i,j-i).toInt();break;
            case 3:az = frame_str.mid(i,j-i).toInt();break;
            case 4:gx = frame_str.mid(i,j-i).toInt();break;
            case 5:gy = frame_str.mid(i,j-i).toInt();
                   gz = frame_str.mid(j+1).toInt();
                   break;
          }
            i=j+1;
            j=frame_str.indexOf(',',i);
            ++k;
        }
        if(6==k){
            tstr.sprintf("adval=%d\nax=%d, ay=%d, az=%d\ngx=%d, gy=%d, gz=%d",
                         adval,ax,ay,az,gx,gy,gz);
            ui->textRecv->append(tstr);
        }

    }
    else if(frame_str.indexOf("7MR")==0&&frame_str.length()>5){
         i=5;
         j=frame_str.indexOf(',',i);
         k=0;
         while(j>0&&k<3){
             switch(k){
             case 0:fvol=frame_str.mid(i,j-i).toDouble();break;
             case 1:fpitch=frame_str.mid(i,j-i).toDouble();break;
             case 2:froll=frame_str.mid(i,j-i).toDouble();
                    fyaw=frame_str.mid(j+1).toDouble();
                    break;
             }
             i=j+1;
             j=frame_str.indexOf(',',i);
             ++k;
         }
         if(3==k){
             tstr.sprintf("vol = %4.1f\npitch= %4.1f°, roll = %4.1f°, yaw = %4.1f°",
                          fvol,fpitch,froll,fyaw);
             ui->textRecv->append(tstr);
         }
    }
}