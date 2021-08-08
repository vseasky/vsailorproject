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
             &&(vRxBuff.length()>=10))
        {
            vHeadCheck();
            //协议处理，如果现有数据大于数据帧长度
            if(thisLength<=vRxBuff.length())//如果现在达到了读取数据量
            {
                if( this->vProtocol.get_protocol_info(
                    this->vProtocol.rx_info.utf8_data,
                    &rx_pos,
                    &this->vProtocol.rx_info.flags_register,
                    this->vProtocol.rx_info.data))
                {
                    //删除已使用
                    vRxBuff.remove(0,thisLength);
                    //获取接收数据的时间戳
                    QString timeString;
                    timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                    /*获取CMDID*/
                    this->vProtocol.rx_info.cmd_id =
                    this->vProtocol.rx_pro.cmd_id;
                    /*获取数据长度*/
                    this->vProtocol.rx_info.float_len =
                    (this->vProtocol.rx_pro.header.data_length-2)/4;
                    /*获取字符长度*/
                    this->vProtocol.rx_info.utf8_data_len =
                    this->vProtocol.rx_date_length;
                    /*加入显示*/
                    vUpdateShowBuff(timeString);
                    /*加入示波器*/
                    this->vRxdata.clear();
                    for(qint8 i=0;i<this->vProtocol.rx_info.float_len;i++)
                    {
                       this->vRxdata.append(this->vProtocol.rx_info.data[i]);
                    }
                    ShowQVariant.setValue(this->vRxdata);
                    emit RxScope(ShowQVariant);
                    //待接收长度清零
                    thisLength = 0;
                    //如果有数据未处理
                    if(vRxBuff.length()>=10)
                    {
                        vSeaskyRxIRQ(vRxBuff);
                    }
                }
                else
                {
                    //解析失败判断
                    thisLength = 0;
                    //删除已使用
                    vRxBuff.remove(0,thisLength);
                    //如果有数据未处理
                    if(!vRxBuff.isEmpty())
                    {
                        //直接当新数据处理
                        vSeaskyRxIRQ(vRxBuff);
                    }
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
         &&(vRxBuff.length()>=10))
    {
        vHeadCheck();
        //协议处理，如果现有数据大于数据帧长度
        if(thisLength<=vRxBuff.length())//如果现在达到了读取数据量
        {
            if(this->vProtocol.get_protocol_info(
               this->vProtocol.rx_info.utf8_data,
               &rx_pos,
               &this->vProtocol.rx_info.flags_register,
               this->vProtocol.rx_info.data))
            {
                //删除已使用
                vRxBuff.remove(0,thisLength);
                //获取接收数据的时间戳
                QString timeString;
                timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                /*获取CMDID*/
                this->vProtocol.rx_info.cmd_id =
                this->vProtocol.rx_pro.cmd_id;
                /*获取数据长度*/
                this->vProtocol.rx_info.float_len =
                (this->vProtocol.rx_pro.header.data_length-2)/4;
                /*获取字符长度*/
                this->vProtocol.rx_info.utf8_data_len =
                this->vProtocol.rx_date_length;
                /*加入显示*/
                vUpdateShowBuff(timeString);
                /*加入示波器*/
                this->vRxdata.clear();
                for(qint8 i=0;i<this->vProtocol.rx_info.float_len;i++)
                {
                   this->vRxdata.append(this->vProtocol.rx_info.data[i]);
                }
                ShowQVariant.setValue(this->vRxdata);
                //待接收长度清零
                thisLength = 0;
                //如果有数据未处理
                if(vRxBuff.length()>=10)
                {
                    vSeaskyRxIRQ(vRxBuff);
                }
            }
            else
            {
                //解析失败判断
                thisLength = 0;
                //删除已使用
                vRxBuff.remove(0,thisLength);
                //如果有数据未处理
                if(!vRxBuff.isEmpty())
                {
                    //直接当新数据处理
                    vSeaskyRxIRQ(vRxBuff);
                }
            }
        }
        return;
    }
}
void vSeaskyPort::vHeadCheck(void)
{
    /************此部分用于将数据连接起来************/
    this->vProtocol.rx_info.utf8_data
           = (uint8_t*)(vRxBuff.data());
    //判断帧头是否正确，并计算出数据总长度
    rx_pos = 0;
    //如果未获取过数据长度
    if(thisLength==0)
    {
        //解析帧头协议获取数据长度
        if(this->vProtocol.get_protocol_len(
               this->vProtocol.rx_info.utf8_data,
               &rx_pos,
               &thisLength))
        {
            //如果超出本软件支持的容量,认为接收错误
            if(thisLength>=this->vRxNumUTF8)
            {
                thisLength = 0;
                vRxBuff.clear();
            }
            return;
        }
        else
        {
            //帧头校验解析失败
            thisLength = 0;
            vRxBuff.clear();
            return;
        }
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
    for(qint16 i=0;i<this->vProtocol.rx_info.float_len;i++)
    {
          this->vRxShow.append(QString("\t%1\t:%2\t%3\n") .
                               arg(this->vRxSeasky.vName[i]).
                               arg(this->vRxSeasky.vQString[i],12).
                               arg(this->vRxSeasky.vUnit[i]));
    }
    this->vRxSeasky.vCmdId = this->vProtocol.rx_info.cmd_id;
    this->vRxSeasky.vReg = this->vProtocol.rx_info.flags_register;
    this->vRxSeasky.vDataLen = this->vProtocol.rx_info.float_len;
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
    if(this->vProtocol.tx_info.data!=nullptr)
    {
        this->vProtocol.tx_info.cmd_id = this->vTxSeasky.vCmdId;
        this->vProtocol.tx_info.flags_register = this->vTxSeasky.vReg;
        this->vProtocol.tx_info.float_len =
                    this->vTxSeasky.vDataLen;
        this->vProtocol.get_protocol_send_data(
                    this->vProtocol.tx_info.cmd_id,
                    this->vProtocol.tx_info.flags_register,
                    this->vProtocol.tx_info.data,
                    this->vProtocol.tx_info.float_len,
                    this->vProtocol.tx_info.utf8_data,
                    &this->vProtocol.tx_info.utf8_data_len);
        vSeaskyTxBuff=QByteArray(reinterpret_cast<const char*>(this->vProtocol.tx_info.utf8_data),
                    this->vProtocol.tx_info.utf8_data_len);
        emit vSerialTx(vSeaskyTxBuff);
    }
}
vSeaskyPortQThread::vSeaskyPortQThread(QObject *parent) : QThread(parent)
{

}
