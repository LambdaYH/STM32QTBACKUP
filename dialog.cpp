#include "dialog.h"
#include "ui_dialog.h"

#include "QtCharts/QChart"
#include "QLineSeries"
#include "QValueAxis"
#include "QTimer"
#include "QTime"
#include "QList"
#include "qmath.h"
#include "QPointF"
#include "QDebug"


//#include <QtCharts/QChartGlobal>
#include "QChartView"
QT_CHARTS_USE_NAMESPACE
static QChart *ADC_chart;
static QLineSeries *ADC_series;
static QLineSeries *fax_series;
static QLineSeries *fay_series;
static QLineSeries *faz_series;
static QLineSeries *ax_series;
static QLineSeries *ay_series;
static QLineSeries *az_series;
static QLineSeries *gx_series;
static QLineSeries *gy_series;
static QLineSeries *gz_series;
//QList<double> dataList;//存储业务数据
static int maxSize = 5000;
//QTimer updateTimer;
static int timeId;
static double fvol =0;
static double fpitch=0,froll=0,fyaw=0;
static int tly_flag=0;
static int ax=0,ay=0,az=0,gx=0,gy=0,gz=0;
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    //查找可用的串口
    ui->comboBox->clear();
    ui->LED->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->KEY->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->textSend->setVisible(false);
    ADC_chart = new QChart;
    QChartView *chartView = new QChartView(ADC_chart);
    ADC_chart->setTitle("ADC|陀螺仪姿态角");
    chartView->setRubberBand(QChartView::RectangleRubberBand);
    setJSVisable();
    ADC_series = new QLineSeries;ADC_series->setName("ADC");
    fax_series = new QLineSeries;fax_series->setName("俯仰角");
    fay_series = new QLineSeries;fay_series->setName("横滚角");
    faz_series = new QLineSeries;faz_series->setName("航向角");
    ax_series = new QLineSeries;ax_series->setName("ax");
    ay_series = new QLineSeries;ay_series->setName("ay");
    az_series = new QLineSeries;az_series->setName("az");
    gx_series = new QLineSeries;gx_series->setName("gx");
    gy_series = new QLineSeries;gy_series->setName("gy");
    gz_series = new QLineSeries;gz_series->setName("gz");
    ADC_chart->addSeries(ADC_series);
    ADC_chart->addSeries(fax_series);
    ADC_chart->addSeries(fay_series);
    ADC_chart->addSeries(faz_series);
    ADC_chart->addSeries(ax_series);
    ADC_chart->addSeries(ay_series);
    ADC_chart->addSeries(az_series);
    ADC_chart->addSeries(gx_series);
    ADC_chart->addSeries(gy_series);
    ADC_chart->addSeries(gz_series);
    for(int i=0;i<maxSize;++i){
       ADC_series->append(i,0);
       fax_series->append(i,0);
       fay_series->append(i,0);
       faz_series->append(i,0);
       ax_series->append(i,0);
       ay_series->append(i,0);
       az_series->append(i,0);
       gx_series->append(i,0);
       gy_series->append(i,0);
       gz_series->append(i,0);
    }
    ADC_series->setUseOpenGL(true);//openGl 加速
    fax_series->setUseOpenGL(true);//openGl 加速
    fay_series->setUseOpenGL(true);//openGl 加速
    faz_series->setUseOpenGL(true);//openGl 加速
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0,maxSize);
    axisX->setLabelFormat("%g");
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(-200,200);

    ADC_chart->setAxisX(axisX,ADC_series);
    ADC_chart->setAxisY(axisY,ADC_series);
    ADC_chart->setAxisX(axisX,fax_series);
    ADC_chart->setAxisY(axisY,fax_series);
    ADC_chart->setAxisX(axisX,fay_series);
    ADC_chart->setAxisY(axisY,fay_series);
    ADC_chart->setAxisX(axisX,faz_series);
    ADC_chart->setAxisY(axisY,faz_series);
    ADC_chart->setAxisX(axisX,ax_series);
    ADC_chart->setAxisY(axisY,ax_series);
    ADC_chart->setAxisX(axisX,ay_series);
    ADC_chart->setAxisY(axisY,ay_series);
    ADC_chart->setAxisX(axisX,az_series);
    ADC_chart->setAxisY(axisY,az_series);
    ADC_chart->setAxisX(axisX,gx_series);
    ADC_chart->setAxisY(axisY,gx_series);
    ADC_chart->setAxisX(axisX,gy_series);
    ADC_chart->setAxisY(axisY,gy_series);
    ADC_chart->setAxisX(axisX,gz_series);
    ADC_chart->setAxisY(axisY,gz_series);


    QVBoxLayout *layout = ui->verticalLayout;
    layout->addWidget(chartView);




//    connect(&updateTimer,SIGNAL(timeout()),this,SLOT(updatedataSlot()));
//    updateTimer.start(0);
    timeId = startTimer(30);
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
    ui->textSend->clear();
}
void Dialog::timerEvent(QTimerEvent *event){
    if(event->timerId()==timeId){//定时器到时间,//模拟数据填充
        static QTime dataTime(QTime::currentTime());
        long int eltime = dataTime.elapsed();
        static int lastpointtime = 0;
        int size = (eltime - lastpointtime);//数据个数
        qint64 sizePoints1=0;
        qint64 sizePoints2=0;
        qint64 sizePoints3=0;
        qint64 sizePoints4=0;
        qint64 sizePoints5=0;
        qint64 sizePoints6=0;
        qint64 sizePoints7=0;
        qint64 sizePoints8=0;
        qint64 sizePoints9=0;
        if(isVisible()){
            QVector<QPointF> oldPoints = ADC_series->pointsVector();//Returns the points in the series as a vector
            QVector<QPointF> points;
            for(int i=size;i<oldPoints.count();++i){
                points.append(QPointF(i-size ,oldPoints.at(i).y()));//替换数据用
            }
            qint64 sizePoints = points.count();
            QVector<QPointF> oldPoints1 = ax_series->pointsVector();
            QVector<QPointF> points1;
            QVector<QPointF> oldPoints2 = ay_series->pointsVector();
            QVector<QPointF> points2;
            QVector<QPointF> oldPoints3 = az_series->pointsVector();
            QVector<QPointF> points3;
            QVector<QPointF> oldPoints4 = gx_series->pointsVector();
            QVector<QPointF> points4;
            QVector<QPointF> oldPoints5 = gy_series->pointsVector();
            QVector<QPointF> points5;
            QVector<QPointF> oldPoints6 = gz_series->pointsVector();
            QVector<QPointF> points6;
            QVector<QPointF> oldPoints7 = fax_series->pointsVector();
            QVector<QPointF> points7;
            QVector<QPointF> oldPoints8 = fay_series->pointsVector();
            QVector<QPointF> points8;
            QVector<QPointF> oldPoints9 = faz_series->pointsVector();
            QVector<QPointF> points9;

            if(tly_flag){
                fax_series->setVisible(true);
                fay_series->setVisible(true);
                faz_series->setVisible(true);
                ax_series->setVisible(false);
                ay_series->setVisible(false);
                az_series->setVisible(false);
                gx_series->setVisible(false);
                gy_series->setVisible(false);
                gz_series->setVisible(false);
                for(int i=size;i<oldPoints7.count();++i){
                    points7.append(QPointF(i-size ,oldPoints7.at(i).y()));//替换数据用
                }
                for(int i=size;i<oldPoints8.count();++i){
                    points8.append(QPointF(i-size ,oldPoints8.at(i).y()));
                }
                for(int i=size;i<oldPoints9.count();++i){
                    points9.append(QPointF(i-size ,oldPoints9.at(i).y()));
                }
                sizePoints7 = points7.count();
                sizePoints8 = points8.count();
                sizePoints9 = points9.count();
            }else{
                fax_series->setVisible(false);
                fay_series->setVisible(false);
                faz_series->setVisible(false);
                ax_series->setVisible(true);
                ay_series->setVisible(true);
                az_series->setVisible(true);
                gx_series->setVisible(true);
                gy_series->setVisible(true);
                gz_series->setVisible(true);
                for(int i=size;i<oldPoints1.count();++i){
                    points1.append(QPointF(i-size ,oldPoints1.at(i).y()));//替换数据用
                }
                for(int i=size;i<oldPoints2.count();++i){
                    points2.append(QPointF(i-size ,oldPoints2.at(i).y()));
                }
                for(int i=size;i<oldPoints3.count();++i){
                    points3.append(QPointF(i-size ,oldPoints3.at(i).y()));
                }
                for(int i=size;i<oldPoints4.count();++i){
                    points4.append(QPointF(i-size ,oldPoints4.at(i).y()));//替换数据用
                }
                for(int i=size;i<oldPoints5.count();++i){
                    points5.append(QPointF(i-size ,oldPoints5.at(i).y()));
                }
                for(int i=size;i<oldPoints6.count();++i){
                    points6.append(QPointF(i-size ,oldPoints6.at(i).y()));
                }
                sizePoints1 = points1.count();
                sizePoints2 = points2.count();
                sizePoints3 = points3.count();
                sizePoints4 = points4.count();
                sizePoints5 = points5.count();
                sizePoints6 = points6.count();
            }
            for(int k=0;k<size;++k){
               points.append(QPointF(k+sizePoints,fvol*100));
                if(tly_flag){
                    points7.append(QPointF(k+sizePoints7,fpitch));
                    points8.append(QPointF(k+sizePoints8,froll));
                    points9.append(QPointF(k+sizePoints9,fyaw));
                }else{
                    points1.append(QPointF(k+sizePoints1,ax/100));
                    points2.append(QPointF(k+sizePoints2,ay/100));
                    points3.append(QPointF(k+sizePoints3,az/100));
                    points4.append(QPointF(k+sizePoints4,gx/100));
                    points5.append(QPointF(k+sizePoints5,gy/100));
                    points6.append(QPointF(k+sizePoints6,gz/100));
                }

            }
            ADC_series->replace(points);
            if(tly_flag){
                fax_series->replace(points7);
                fay_series->replace(points8);
                faz_series->replace(points9);
            }else{
                ax_series->replace(points1);
                ay_series->replace(points2);
                az_series->replace(points3);
                gx_series->replace(points4);
                gy_series->replace(points5);
                gz_series->replace(points6);
            }

            lastpointtime = eltime;
       }
    }
}
void Dialog::setYSVisable()
{
    ui->fax->setVisible(false);
    ui->fay->setVisible(false);
    ui->faz->setVisible(false);
    ui->fax_l->setVisible(false);
    ui->fay_l->setVisible(false);
    ui->faz_l->setVisible(false);
    ui->ax->setVisible(true);
    ui->ay->setVisible(true);
    ui->az->setVisible(true);
    ui->gx->setVisible(true);
    ui->gy->setVisible(true);
    ui->gz->setVisible(true);
    ui->ax_l->setVisible(true);
    ui->ay_l->setVisible(true);
    ui->az_l->setVisible(true);
    ui->gx_l->setVisible(true);
    ui->gy_l->setVisible(true);
    ui->gz_l->setVisible(true);
    tly_flag=0;
    ADC_chart->setTitle("ADC|陀螺仪原始数据");

}

void Dialog::setJSVisable()
{
    ui->fax->setVisible(true);
    ui->fay->setVisible(true);
    ui->faz->setVisible(true);
    ui->fax_l->setVisible(true);
    ui->fay_l->setVisible(true);
    ui->faz_l->setVisible(true);
    ui->ax->setVisible(false);
    ui->ay->setVisible(false);
    ui->az->setVisible(false);
    ui->gx->setVisible(false);
    ui->gy->setVisible(false);
    ui->gz->setVisible(false);
    ui->ax_l->setVisible(false);
    ui->ay_l->setVisible(false);
    ui->az_l->setVisible(false);
    ui->gx_l->setVisible(false);
    ui->gy_l->setVisible(false);
    ui->gz_l->setVisible(false);
    tly_flag=1;
    ADC_chart->setTitle("ADC|陀螺仪姿态角");

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
        ui->ax->append(frame_str+"check sum error!\n");
        return;
    }

    frame_str=frame_str.left(frame_str.length()-2);

    int i,j,k;
    int led_sta[4],btn_sta[4];
    int adval = 0;
    QString LED_str;
    QString KEY_str;
    QString tstr;
    QString tmp;

    if(frame_str.indexOf("7MBD")==0 && frame_str.length()>=12){
        for(i=0;i<4;i++){
            led_sta[i]=(frame_str[4+i]=='0')?0:1;
        }
        for(i=0;i<4;i++){
            btn_sta[i] = (frame_str[8+i]=='U')?0:1;
        }
        LED_str.sprintf("%s   %s   %s   %s",led_sta[0]?"💡":"⚪",led_sta[1]?"💡":"⚪",led_sta[2]?"💡":"⚪",led_sta[3]?"💡":"⚪");
        KEY_str.sprintf("%s   %s   %s   %s",btn_sta[0]?"▁":"█",btn_sta[1]?"▁":"█",btn_sta[2]?"▁":"█",btn_sta[3]?"▁":"█");
        ui->LED->setText(LED_str);
        ui->KEY->setText(KEY_str);
    }
    else if(frame_str.indexOf("7MY,")==0 && frame_str.length()>5){
        i=5;
        j=frame_str.indexOf(',',i);
        k=0;
        setYSVisable();
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

            ui->ax->setText(QString::number(ax));
            ui->ay->setText(QString::number(ay));
            ui->az->setText(QString::number(az));
            ui->gx->setText(QString::number(gx));
            ui->gy->setText(QString::number(gy));
            ui->gz->setText(QString::number(gz));
            ui->ADC->setText(QString::number(adval));


        }

    }
    else if(frame_str.indexOf("7MR")==0&&frame_str.length()>5){
         i=5;
         j=frame_str.indexOf(',',i);
         k=0;
         setJSVisable();
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
             tmp.sprintf("%4.1f",fpitch);
             ui->fax->setText(tmp);
             tmp.sprintf("%4.1f",froll);
             ui->fay->setText(tmp);
             tmp.sprintf("%4.1f",fyaw);
             ui->faz->setText(tmp);
             tmp.sprintf("%4.1f",fvol);
             ui->ADC->setText(QString::number(fvol));

         }
    }
}
