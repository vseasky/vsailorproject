#include "vserialport.h"
/*
 *
 *
 *该文件主要提供可以实现的串口调用接口，以及需要的所有调试信息，如果其他地方需要建议使用指针方式复用，但是需要避免同时操作写
 *
 */
#include <QThread>
#include <QDebug>
vSerialPort::vSerialPort(QObject *parent) : QObject(parent)
{
    qDebug()<<"main tid:vSerialPort"<< QThread::currentThreadId();
    this->qSerial        = new QSerialPort;
    this->vSerialConfig  = new vSerialConfigStruct;
    this->vSerialData    = new vSerialDataStruct;
    this->vCodeConver    = new vSerialCodeModeEnum;
}
vSerialPort::~vSerialPort(void)
{
    delete this->qSerial;
    delete this->vSerialConfig;
    delete this->vSerialData;
    delete this->vCodeConver;
    delete [] this->vSerialComInfo;
};
//通过相关事件触发此函数，进行串口设备查询，并更新可用设备列表
void vSerialPort::vSerialComScanf(void)
{
    qint32 SerialComCnt  = 0;
    this->vSerialPortCnt = 0;
    //要删除原有的内存空间
    if(this->vSerialComInfo!=nullptr)
    {
        delete [] this->vSerialComInfo;
    }
    //获取串口设备数量
    this->vSerialPortCnt = QSerialPortInfo::availablePorts().length();
    this->vSerialComInfo = new vSerialComInfoStruct[this->vSerialPortCnt];
    SerialComCnt = 0;
    foreach(const QSerialPortInfo &port,QSerialPortInfo::availablePorts())
    {
        this->vSerialComInfo[SerialComCnt].SerialName      = port.portName();
        this->vSerialComInfo[SerialComCnt].ProductCode     = port.productIdentifier();
        this->vSerialComInfo[SerialComCnt].SyestemPosition = port.systemLocation();
        this->vSerialComInfo[SerialComCnt].SerialNumStr    = port.serialNumber();
        this->vSerialComInfo[SerialComCnt].DescribeStr     = port.description();
        this->vSerialComInfo[SerialComCnt].Manufacturer    = port.manufacturer();
        this->vSerialComInfo[SerialComCnt].SupplierCode    = port.vendorIdentifier();
        SerialComCnt++;
    }
}
//打开选中的串口设备
void vSerialPort::vSerialOpen(bool & isOpen)
{
    //关闭串口
    this->vSerialClose();
    //设置串口名称
    this->qSerial->setPortName(this->vSerialConfig->vSerialPortName);
    //设置串口波特率
    this->qSerial->setBaudRate(this->vSerialConfig->vSerialBaudRate);
    //设置停止位
    this->qSerial->setStopBits(this->vSerialConfig->vSerialStopBits);
    //设置数据位
    this->qSerial->setDataBits(this->vSerialConfig->vSerialDataBits);
    //设置奇偶校验
    this->qSerial->setParity(this->vSerialConfig->vSerialParrity);
    //设置数据流控
    this->qSerial->setFlowControl(this->vSerialConfig->vSerialFlowControl);
    //打开串口设备
    isOpen =  (this->qSerial->open(QIODevice::ReadWrite));
}
void vSerialPort::vSerialClose(void)
{
    //关闭串口
    if(this->qSerial->isOpen())
    {
        this->qSerial->close();
    }
}
///*同一时间只允许一个线程调用*/
void vSerialPort::vWriteData(const QByteArray &str)
{
    if(this->qSerial->isOpen())
    {
        //统计
        this->vSerialData->txByteCnt+=str.length();
        this->qSerial->write(str);
    }
    else
    {
        emit vTxError();
    }
}
//清除统计的数据量
void vSerialPort::vClearCntNum(void)
{
    this->vSerialData->txByteCnt = 0;
    this->vSerialData->rxByteCnt = 0;
}
void vSerialPort::vSerialError(QSerialPort::SerialPortError serialPortError)
{

};
vSerialThread::vSerialThread(QObject *parent) : QThread(parent)
{

}
