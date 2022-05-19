#include "vseaskyport.h"
#include <QDateTime>
//使用前务必给的相关参数地址信息
const qint32 rxnamewidth = 64;
const qint32 rxunitwidth = 64;
const qint32 txnamewidth = 64;
const qint32 txunitwidth = 64;
const qint32 floatnum    = 6;  //小数点位数



vSeaskyPort::vSeaskyPort(QWidget *parent) : QObject(parent)
{
    vQTimerTxStop();
    connect(&this->vQTimerTx,&QTimer::timeout,
            this,&vSeaskyPort::vSeaskyTxSlot);
}
void vSeaskyPort::vConnectRx(void)
{
    void(vSeaskyPort:: * rxSlot)(void) =
            &vSeaskyPort::vSeaskyRxIRQ;
    connect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,rxSlot,Qt::QueuedConnection);
}
void vSeaskyPort::vDisConnectRx(void)
{
    void(vSeaskyPort:: * rxSlot)(void) = &vSeaskyPort::vSeaskyRxIRQ;
    disconnect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,rxSlot);
}
void vSeaskyPort::vConnectTx(void)
{
    connect(this,&vSeaskyPort::vSerialTx,
            this->vSerial,&vSerialPort::vWriteData
            ,Qt::QueuedConnection);
}
void vSeaskyPort::vDisConnectTx(void)
{
    disconnect(this,&vSeaskyPort::vSerialTx,
            this->vSerial,&vSerialPort::vWriteData);
}
void vSeaskyPort::setQWidgetAddr(QWidget * addrTx,QWidget * addrRx)
{
   vTxEdit = addrTx;
   vRxEdit = addrRx;
}
void vSeaskyPort::setRxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF)
{
    this->vRxSeasky.vQString = strF;
    this->vRxSeasky.vName = strN;
    this->vRxSeasky.vUnit = strU;
    this->vRxSeasky.vFloat = addrF;
}
void vSeaskyPort::setTxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF)
{
    this->vTxSeasky.vQString = strF;
    this->vTxSeasky.vName = strN;
    this->vTxSeasky.vUnit = strU;
    this->vTxSeasky.vFloat = addrF;
}


void vSeaskyPort::configQWidgetRx(qint32 num)
{
    this->vRxNum = num;
    this->vRxNumUTF8 = num*4+10;
    QVBoxLayout * vQVBoxLayout = new QVBoxLayout(this->vRxEdit);
    QLineEdit   * vlineEdit1;
    QLineEdit   * vlineEdit2;
    QLineEdit   * vlineEdit3;
    for(qint16 i=0;i<this->vRxNum;i++)
    {
        QWidget     * vQWidget = new QWidget(this->vRxEdit);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vlineEdit1 = new QLineEdit(vQWidget);
        vlineEdit2 = new QLineEdit(vQWidget);
        vlineEdit3 = new QLineEdit(vQWidget);
        vlineEdit1->setMinimumSize(rxnamewidth,24);
        vlineEdit1->setMaximumSize(rxnamewidth,24);
        vlineEdit2->setMinimumSize(0,24);
        vlineEdit2->setMaximumSize(16777215,24);
        vlineEdit2->setAlignment(Qt::AlignLeft);
        vlineEdit2->setReadOnly(true);
        vlineEdit2->setValidator(new QDoubleValidator(vlineEdit2));
        vlineEdit3->setMinimumSize(rxunitwidth,24);
        vlineEdit3->setMaximumSize(rxunitwidth,24);
        vlineEdit1->setStyleSheet("background-color:#FFFFFF;");
        vlineEdit2->setStyleSheet("background-color:#CCFFFF;"
                                  "color:#FF0033;"
                                  "font-weight: bold;");
        vlineEdit3->setStyleSheet("background-color:#FFFFFF;");
        vQHBoxLayout->addWidget(vlineEdit1);
        vQHBoxLayout->addWidget(vlineEdit2);
        vQHBoxLayout->addWidget(vlineEdit3);
        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);
        //数据显示更新
        connect(&vQTimer,&QTimer::timeout,[=]()
        {
            //默认显示六位有效数字
            this->vRxSeasky.vQString[i] = QString::number((this->vRxSeasky.vFloat[i]),'f',floatnum);
            this->vRxSeasky.vQString[i].remove(QRegExp("0*$"));
            this->vRxSeasky.vQString[i].remove(QRegExp("[.]$"));
            vlineEdit2->setText(this->vRxSeasky.vQString[i]);
        });
        connect(vlineEdit1,&QLineEdit::editingFinished,[=]()
        {
            this->vRxSeasky.vName[i] = vlineEdit1->text();
            emit vInfoChanged();
        });
        connect(vlineEdit3,&QLineEdit::editingFinished,[=]()
        {
            this->vRxSeasky.vUnit[i] = vlineEdit3->text();
            emit vInfoChanged();
        });
        connect(this,&vSeaskyPort::vQWidgetRxShow,[=]()
        {
            vlineEdit1->setText(this->vRxSeasky.vName[i]);
            vlineEdit2->setText(this->vRxSeasky.vQString[i]);
            vlineEdit3->setText(this->vRxSeasky.vUnit[i]);
        });
    }
}
void vSeaskyPort::configQWidgetTx(qint32 num)
{
    this->vTxNum     = num;
    this->vTxNumUTF8 = num*4+10;
    QVBoxLayout * vQVBoxLayout = new QVBoxLayout(this->vTxEdit);
    QLineEdit   * vlineEdit1;
    QLineEdit   * vlineEdit2;
    QLineEdit   * vlineEdit3;
    for(qint16 i=0;i<this->vTxNum;i++)
    {
        QWidget     * vQWidget = new QWidget(this->vTxEdit);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vlineEdit1 = new QLineEdit(vQWidget);
        vlineEdit2 = new QLineEdit(vQWidget);
        vlineEdit3 = new QLineEdit(vQWidget);
        vlineEdit1->setMinimumSize(txnamewidth,24);
        vlineEdit1->setMaximumSize(txnamewidth,24);
        vlineEdit2->setMinimumSize(0,24);
        vlineEdit2->setMaximumSize(16777215,24);
        QRegExp txFloat("^(-?[0]|-?[1-9][0-9]{1,6})(?:\\.\\d{1,6})?$|(^\\t?$)");
        vlineEdit2->setValidator(new QRegExpValidator(txFloat,vlineEdit2));
        vlineEdit3->setMinimumSize(txunitwidth,24);
        vlineEdit3->setMaximumSize(txunitwidth,24);
        vlineEdit1->setStyleSheet("background-color:#FFFFFF;");
        vlineEdit2->setStyleSheet("background-color:#FFFFFF;");
        vlineEdit3->setStyleSheet("background-color:#FFFFFF;");

        vQHBoxLayout->addWidget(vlineEdit1);
        vQHBoxLayout->addWidget(vlineEdit2);
        vQHBoxLayout->addWidget(vlineEdit3);
        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);
        connect(vlineEdit1,&QLineEdit::editingFinished,[=]()
        {
            this->vTxSeasky.vName[i] = vlineEdit1->text();
            emit vInfoChanged();
        });
        connect(vlineEdit2,&QLineEdit::editingFinished,[=]()
        {
            this->vTxSeasky.vQString[i] = vlineEdit2->text();
            this->vTxSeasky.vFloat[i] = this->vTxSeasky.vQString[i].toDouble();
            emit vInfoChanged();
        });
        connect(vlineEdit3,&QLineEdit::editingFinished,[=]()
        {
            this->vTxSeasky.vUnit[i] = vlineEdit3->text();
            emit vInfoChanged();
        });
        connect(this,&vSeaskyPort::vQWidgetTxShow,[=]()
        {
            vlineEdit1->setText(this->vTxSeasky.vName[i]);
            vlineEdit2->setText(this->vTxSeasky.vQString[i]);
            vlineEdit3->setText(this->vTxSeasky.vUnit[i]);
        });
    }
}
void vSeaskyPort::timerStart(void)
{
    vQTimerEnable = true;
    if(!this->vQTimer.isActive())
    {
        this->vQTimer.start(this->timerCntSet);
    }
}
void vSeaskyPort::timerStop(void)
{
    vQTimerEnable = false;
    if(this->vQTimer.isActive())
    {
         this->vQTimer.stop();
    }
}

//由于追求极高的解析效率，嵌入式端尽可能打包发送，异常断帧情况将极大的影响程序效率，甚至于造成程序崩溃
void vSeaskyPort::vSeaskyRxIRQ(void)
{
    QByteArray vRxSerialBuff;
    if(this->vSerial!=nullptr)
    {
        if(this->vSerial->qSerial->isOpen())
        {
            vRxSerialBuff = this->vSerial->qSerial->readAll();
        }
        if(vRxSerialBuff.isEmpty())
        {
            return;
        }
        //检验数据帧头,帧头固定为(0XA5),同时务必确认帧头与上一帧数据有时差，协议容错
        if((!vRxSerialBuff.isEmpty())&&
            (vRxSerialBuff.at(0)==char(0XA5)))
        {
            vRxBuff = vRxSerialBuff;
        }
        else if((!vRxBuff.isEmpty())&&
                 (vRxBuff.at(0)==char(0XA5)))
        {
            vRxBuff.append(vRxSerialBuff);
        }
        else
        {
            return;
        }
        //数据帧协议解析，以及判断，需要先具备10个字符，即包含完整的帧头，帧尾数据
        if((vRxBuff.at(0)==char(0XA5))
             &&(vRxBuff.length()>=12))
        {
                //协议处理，如果现有数据大于数据帧长度
                memcpy(&this->vProtocol.pRxProtocol->message_st.pData[0],&((uint8_t*)(vRxBuff.data()))[0],vRxBuff.length());
                if(parse_protocol(this->vProtocol.pRxProtocol,vRxBuff.length())==PROTOCOL_RESULT_OK)
                {
                    for(uint8_t k = 0;k<this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.data_len;k++)
                    {
                        qDebug()<<"pData:"<<this->vProtocol.pRxProtocol->message_st.pData[k];
                    }
                    //获取接收数据的时间戳
                    QString timeString;
                    timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                    /*加入显示*/
                    vUpdateShowBuff(timeString);
                    qDebug()<<this->vProtocol.pRxProtocol->frame_st.frame_user.equipment_type;
                    qDebug()<<this->vProtocol.pRxProtocol->frame_st.frame_user.equipment_id;
                    qDebug()<<this->vProtocol.pRxProtocol->frame_st.frame_user.data_id;
                    /*加入示波器*/
                    this->vRxdata.clear();
                    for(qint8 i=0;i<this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.data_len;i++)
                    {
                        qDebug()<<((float*)this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.pData)[i];
                        this->vRxdata.append(((float*)this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.pData)[i]);
                    }
                    ShowQVariant.setValue(this->vRxdata);
                    emit RxScope(ShowQVariant);
                    //删除已使用
                    vRxBuff.remove(0,this->vProtocol.pRxProtocol->message_st.data_len);
                    //待接收长度清零
                    this->vProtocol.pRxProtocol->message_st.data_len = 0;
                    //如果有数据未处理
                    if(vRxBuff.length()>=12)
                    {
                        vSeaskyRxIRQ(vRxBuff);
                    }
                }
            return;
        }
    }
}
//由于追求极高的解析效率，嵌入式端尽可能打包发送，异常断帧情况将极大的影响程序效率，甚至于造成程序崩溃
void vSeaskyPort::vSeaskyRxIRQ(const QByteArray &str)
{
    //检验数据帧头,帧头固定为(0XA5),同时务必确认帧头与上一帧数据有时差
    if((str.at(0)==char(0XA5)))
    {
        vRxBuff = str;
    }
    else if(vRxBuff.at(0)==char(0XA5))
    {
        vRxBuff.append(str);
    }
    //数据帧协议解析，以及判断，需要先具备10个字符，即包含完整的帧头，帧尾数据
    if((vRxBuff.at(0)==char(0XA5))
         &&(vRxBuff.length()>=12))
    {
        //协议处理，如果现有数据大于数据帧长度
        memcpy(&this->vProtocol.pRxProtocol->message_st.pData[0],&((uint8_t*)(vRxBuff.data()))[0],vRxBuff.length());
        if(parse_protocol(this->vProtocol.pRxProtocol,vRxBuff.length())==PROTOCOL_RESULT_OK)
        {
                //获取接收数据的时间戳
                QString timeString;
                timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                /*获取CMDID*/
                /*加入显示*/
                vUpdateShowBuff(timeString);
                /*加入示波器*/
                this->vRxdata.clear();
                for(qint8 i=0;i<this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.data_len;i++)
                {
                    qDebug()<<((float*)this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.pData)[i];
                   this->vRxdata.append(((float*)this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.pData)[i]);
                }
                //删除已使用
                vRxBuff.remove(0,this->vProtocol.pRxProtocol->message_st.data_len);
                //待接收长度清零
                this->vProtocol.pRxProtocol->message_st.data_len = 0;
                //如果有数据未处理
                if(vRxBuff.length()>=12)
                {
                    vSeaskyRxIRQ(vRxBuff);
                }
            }
        return;
    }
}
void vSeaskyPort::setPlainEdit(vPlainTextEdit * edit)
{
    static bool hexEnable = false;
    this->vPlainEdit = edit;
    this->vPlainEdit->SetShowBuffAddr(&this->vRxShow);
    this->vPlainEdit->hexEnable = &hexEnable;
    this->vPlainEdit->TimerStart();
};
void vSeaskyPort::vUpdateShowBuff(const QString &currentTimer)
{
    this->vRxShow.append(currentTimer);
    for(qint16 i=0;i<this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.data_len;i++)
    {
          this->vRxShow.append(QString("\t%1\t:%2\t%3\n") .
                               arg(this->vRxSeasky.vName[i]).
                               arg(this->vRxSeasky.vQString[i],12).
                               arg(this->vRxSeasky.vUnit[i]));
    }
    this->vRxSeasky.vCmdId      = this->vProtocol.pRxProtocol->frame_st.frame_user.equipment_id;
    this->vRxSeasky.vReg        = this->vProtocol.pRxProtocol->frame_st.frame_user.data_id;
    this->vRxSeasky.vDataLen    = this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.data_len;
    emit showRxHead();
    emit textChanged();
}
void vSeaskyPort::vQTimerTxStart(void)
{
    if(!this->vQTimerTx.isActive())
    {
        this->vQTimerTx.start(this->vQtimerTxCnt);
    }
}
void vSeaskyPort::vQTimerTxStop(void)
{
    if(this->vQTimerTx.isActive())
    {
        this->vQTimerTx.stop();
    }
}
void vSeaskyPort::vSeaskyTxSlot(void)
{
    if(this->vTxSeasky.vDataLen>this->vTxNum)
    {
        return;//超出长度，错误
    }
    if(this->vProtocol.pTxProtocol->message_st.pData!=nullptr)
    {
        //vSeaskyPortV2
        this->vProtocol.pTxProtocol->frame_st.frame_user.equipment_type = 0x01;
        this->vProtocol.pTxProtocol->frame_st.frame_user.equipment_id = this->vTxSeasky.vCmdId;
        this->vProtocol.pTxProtocol->frame_st.frame_user.data_id = this->vTxSeasky.vReg;
        this->vProtocol.pTxProtocol->frame_st.frame_user.cmd_data.data_len = this->vTxSeasky.vDataLen;
        make_protocol(this->vProtocol.pTxProtocol);
        vSeaskyTxBuff=QByteArray(reinterpret_cast<const char*>(this->vProtocol.pTxProtocol->message_st.pData),
                    this->vProtocol.pTxProtocol->message_st.data_len);
        emit vSerialTx(vSeaskyTxBuff);
    }
}
vSeaskyPortQThread::vSeaskyPortQThread(QObject *parent) : QThread(parent)
{

}
