#ifndef VSERIALPORT_H
#define VSERIALPORT_H

#include <QObject>
/*
 *<QSerialPort>:提供串口相关的服务
 *<QSerialPortInfo>:提供串口设备查询的服务
 *<QThread>:为串口创建线程
 */
#include <QSerialPort>
#include <QSerialPortInfo>
#include <vcodeconverter.h>
#include <QThread>

#include <QDebug>

//获取串口设备信息列表，以便于识别正确设备
typedef struct
{                           //描述:        eg
    QString SerialName;     //名称:        "COMx"
    qint16  ProductCode;    //产品编号:     "29987"
    QString SyestemPosition;//系统位置:     "\\\\.\\COM10"
    QString SerialNumStr;   //序列号字符串: ""
    QString DescribeStr;    //描述字符串:   "USB-SERIAL CH340"
    QString Manufacturer;   //制造商:      "wch.cn"
    QString SupplierCode;   //供应商编码:   "6790"
}vSerialComInfoStruct;

//串口操作选中对象及初始化赋值
typedef struct
{
    QString                  vSerialPortName;   //串口名称
    qint32                   vSerialBaudRate;   //串口波特率
    QSerialPort::StopBits    vSerialStopBits;   //串口停止位
    QSerialPort::DataBits    vSerialDataBits;   //串口数据位
    QSerialPort::Parity      vSerialParrity;    //设置奇偶校验
    QSerialPort::FlowControl vSerialFlowControl;//数据流控
}vSerialConfigStruct;

//数据处理及其解析相关
typedef struct
{
    //缓存数据
    QByteArray RxBuff;   //原始数据收发缓冲
//    QByteArray HexBroserBuff;   //十六进制格式浏览器缓冲
//    QByteArray BrowserBuff;     //浏览器缓冲
//    QByteArray UnShowedRxBuff;  //上次未上屏数据
    //收发数据统计
    qint64 rxByteCnt = 0;       //接收数据统计
    qint64 txByteCnt = 0;       //发送数据统计
    //控制标志
    bool rxHexEnable = false;       //16进制格式接收使能
    bool txHexEnable = false;       //16进制格式发送使能
    bool rxTimeStampEnable = false; //接收数据加入时间戳使能
}vSerialDataStruct;
class vSerialPort : public QObject
{
     Q_OBJECT
public:
    explicit vSerialPort(QObject *parent = nullptr);
    ~vSerialPort(void);
    /*以下信息将提供给很多接口使用，地址初始化时提供，外部无需再次赋值*/
    QSerialPort          * qSerial = nullptr;                //提供串口操作
    vSerialComInfoStruct * vSerialComInfo=nullptr; //串口设备扫描列表
    vSerialConfigStruct  * vSerialConfig = nullptr;          //要使用的串口信息
    vSerialDataStruct    * vSerialData = nullptr;            //简单处理后的数据
    vSerialCodeModeEnum  * vCodeConver = nullptr;            //编码格式信息
    qint32                 vSerialPortCnt;         //识别到的串口设备数量
public slots:
    void vSerialOpen(bool & isOpen);
    void vSerialClose(void);    //关闭串口设备
    void vWriteData(const QByteArray &str);
    void vSerialComScanf(void); //获取串口列表信息
    void vSerialError(QSerialPort::SerialPortError serialPortError);//串口异常
    void vClearCntNum(void);//清除数据统计
signals:
    void vTxError(void);
};
class vSerialThread : public QThread
{
    Q_OBJECT
public:
    explicit vSerialThread(QObject *parent = nullptr);
public:
    void run(void)
    {
        qDebug()<<"main tid:vSerialThread run"<< QThread::currentThreadId();
        exec();
    }
signals:
public slots:
};

#endif // VSERIALPORT
