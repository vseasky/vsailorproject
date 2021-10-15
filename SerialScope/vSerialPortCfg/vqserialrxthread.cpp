#include "vqserialrxthread.h"
#include "vansibash.h"
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
    static int haveLine=false;
    QByteArray vRxSerialBuff;
    static QByteArray vReadAll;
    int    start_line = -1;
    if(this->vSerial!=NULL)
    {
        if(this->vSerial->qSerial->isOpen())
        {
            vReadAll.append(this->vSerial->qSerial->readAll());
            //获取"\n"的下标
            start_line = vReadAll.lastIndexOf("\n");
            if(this->vSerial->vSerialData->rxHexEnable)
            {
                vRxSerialBuff.append(vReadAll);
                vReadAll.clear();
            }
            else if(start_line>=0)
            {
                vAnsiBash vAnsiBash_t;
                //bool      isConvertio=false;
                //带"\n"一起复制进去
                vRxSerialBuff.append(vReadAll.mid(0,start_line+1));
                //vAnsiBash_t.vAnsiColor(&vRxSerialBuff,&isConvertio);
                this->vSerial->vSerialData->rxByteCnt += vRxSerialBuff.length();
                vReadAll.remove(0,start_line+1);
            }
        }
        if(vRxSerialBuff.isEmpty())
        {
            return;
        }
        //接收添加到缓冲
        if(this->vSerial->vSerialData->rxHexEnable)
        {
            //二进制格式，不做处理
            this->vSerial->vSerialData->RxBuff.append((vRxSerialBuff));
        }
        else
        {
            //是否加入时间戳
            if(!this->vSerial->vSerialData->rxTimeStampEnable)
            {
                 this->vSerial->vSerialData->RxBuff.append(QString(CodeDecoding(vRxSerialBuff)));
            }
            else
            {
                QString timeString;
                timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                //去掉\r ,\n替换为<br/>
                this->vSerial->vSerialData->RxBuff.append(
                  QString(CodeDecoding(vRxSerialBuff))
                            .replace("\r","")
                            .replace("\n",timeString));
            }
        }
    }
}
vQserialRxThread::vQserialRxThread(QObject *parent) : QThread(parent)
{

}
