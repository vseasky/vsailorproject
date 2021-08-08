#include "qserialportcft.h"
#include <QDebug>
//qSerialPortCft::qSerialPortCft(QObject *parent) : QObject(parent)
//{
//    this->vSerial       = new QSerialPort;
//    this->vSerialConfig = new vSerialConfigStruct;
//    vSerialComScanf();
//}
//qSerialPortCft::~qSerialPortCft()
//{
//    delete this->vSerial;
//    delete this->vSerialConfig;
//    delete  [] this->vSerialComInfo;
//}
////通过相关事件触发此函数，进行串口设备查询，并更新可用设备列表
//void qSerialPortCft::vSerialComScanf(void)
//{
//    qint32 SerialComCnt  = 0;
//    this->vSerialPortCnt = 0;
//    this->vSerialPortCnt = 0;
//    //要删除原有的内存空间
//    if(this->vSerialComInfo!=nullptr)
//    {
//        delete [] this->vSerialComInfo;
//    }
//    //获取串口设备数量
//    this->vSerialPortCnt = QSerialPortInfo::availablePorts().length();
//    this->vSerialComInfo = new vSerialComInfoStruct[this->vSerialPortCnt];
//    SerialComCnt= 0;
//    foreach(const QSerialPortInfo &port,QSerialPortInfo::availablePorts())
//    {
//        this->vSerialComInfo[SerialComCnt].SerialName      = port.portName();
//        this->vSerialComInfo[SerialComCnt].ProductCode     = port.productIdentifier();
//        this->vSerialComInfo[SerialComCnt].SyestemPosition = port.systemLocation();
//        this->vSerialComInfo[SerialComCnt].SerialNumStr    = port.serialNumber();
//        this->vSerialComInfo[SerialComCnt].DescribeStr     = port.description();
//        this->vSerialComInfo[SerialComCnt].Manufacturer    = port.manufacturer();
//        this->vSerialComInfo[SerialComCnt].SupplierCode    = port.vendorIdentifier();
//        qDebug()<<this->vSerialComInfo[SerialComCnt].SerialName;
//        SerialComCnt++;
//    }
//}
////打开选中的串口设备
//bool qSerialPortCft::vSerialOpen(void)
//{
//    //关闭串口
//    this->vSerialClose();
//    //设置串口名称
//    this->vSerial->setPortName(this->vSerialConfig->vSerialPortName);
//    //设置串口波特率
//    this->vSerial->setBaudRate(this->vSerialConfig->vSerialBaudRate);
//    //设置数据位
//    this->vSerial->setDataBits(this->vSerialConfig->vSerialDataBits);
//    //设置奇偶校验
//    this->vSerial->setParity(this->vSerialConfig->vSerialParrity);
//    //设置数据流控
//    this->vSerial->setFlowControl(this->vSerialConfig->vSerialFlowControl);
//    //打开串口设备
//    return (this->vSerial->open(QIODevice::ReadWrite));
//}
//void qSerialPortCft::vSerialClose(void)
//{
//    //关闭串口
//    if(this->vSerial->isOpen())
//    {
//        this->vSerial->close();
//    }
//}
