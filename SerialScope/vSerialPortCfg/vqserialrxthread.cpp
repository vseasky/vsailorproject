#include "vqserialrxthread.h"
vQObjectRx::vQObjectRx(QObject *parent) : QObject(parent)
{

}
vQObjectRx::~vQObjectRx(void)
{

};
//打开接收的信号与槽，方便切换
void vQObjectRx::vConnectRx(void)
{
    connect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,&vQObjectRx::vReadSlot,Qt::QueuedConnection);
}
//断开接收的信号与槽，方便切换
void vQObjectRx::vDisConnectRx(void)
{
    disconnect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,&vQObjectRx::vReadSlot);
}
void vQObjectRx::vReadSlot(void)
{
    QByteArray vRxSerialBuff;
    if(this->vSerial!=NULL)
    {
        if(this->vSerial->qSerial->isOpen())
        {
            vRxSerialBuff = this->vSerial->qSerial->readAll();
            this->vSerial->vSerialData->rxByteCnt += vRxSerialBuff.length();
        }
        if(vRxSerialBuff.isEmpty())
        {
            return;
        }
        if(this->vSerial->vSerialData->rxHexEnable)
        {
            this->vSerial->vSerialData->RxBuff.append((vRxSerialBuff));
        }
        else
        {
            if(!this->vSerial->vSerialData->rxTimeStampEnable)
            {
                this->vSerial->vSerialData->RxBuff.append(QString(CodeDecoding(vRxSerialBuff)).replace("\r",""));
            }
            else
            {
                QString timeString;
                timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                this->vSerial->vSerialData->RxBuff.append(QString(CodeDecoding(vRxSerialBuff)).replace("\r\n",timeString));
            }
        }
    }
}
vQserialRxThread::vQserialRxThread(QObject *parent) : QThread(parent)
{

}
