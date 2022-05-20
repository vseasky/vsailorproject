#include "vseaskyport.h"
#include <QDateTime>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
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
void vSeaskyPort::vConnect(void)
{
    void(vSeaskyPort:: * rxSlot)(void) =
            &vSeaskyPort::vSeaskyRxIRQ;
    connect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,rxSlot,Qt::QueuedConnection);
    connect(this,&vSeaskyPort::vSerialTx,
            this->vSerial,&vSerialPort::vWriteData
            ,Qt::QueuedConnection);
}
void vSeaskyPort::vDisConnect(void)
{
    void(vSeaskyPort:: * rxSlot)(void) = &vSeaskyPort::vSeaskyRxIRQ;
    disconnect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,rxSlot);
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
        connect(vlineEdit1,&QLineEdit::textChanged,[=]()
        {
            this->vRxSeasky.vName[i] = vlineEdit1->text();
            emit vInfoChanged();
        });
        connect(vlineEdit3,&QLineEdit::textChanged,[=]()
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
    QCheckBox   * vQCheckBox;
    QPushButton * vQPushButton;
    QWidget     * vQWidget;
    QHBoxLayout * vQHBoxLayout;


    vQCheckBox = new QCheckBox();
    vQPushButton = new QPushButton();
    vQWidget = new QWidget(this->vTxEdit);
    vQHBoxLayout = new QHBoxLayout(vQWidget);
    vlineEdit1 = new QLineEdit(vQWidget);



    QHBoxLayout * vQHBoxLayout1 = new QHBoxLayout();
    QWidget     * vQWidget1 = new QWidget();

    vQPushButton->setMaximumSize(32,24);
    vQHBoxLayout->setContentsMargins(0,0,0,0);
    vQHBoxLayout->setSpacing(0);
    vQHBoxLayout->setMargin(0);
    vQHBoxLayout->setAlignment(Qt::AlignLeft);



    vDataAutoTxEnable = vQCheckBox->isChecked();

    connect(vQCheckBox,&QCheckBox::released,[=](){
        vDataAutoTxEnable = vQCheckBox->isChecked();
    });

    connect(vQPushButton,&QPushButton::released,[=](){
        {
            this->vProtocol.pTxProtocol->frame_st.frame_user.equipment_type = this->vTxSeasky.vEquipmentType;
            this->vProtocol.pTxProtocol->frame_st.frame_user.equipment_id = this->vTxSeasky.vEquipmentId;
            this->vProtocol.pTxProtocol->frame_st.frame_user.data_id = this->vTxSeasky.vDataId;
            this->vProtocol.pTxProtocol->frame_st.frame_user.cmd_data.data_len = this->vTxSeasky.vDataLen;
            make_protocol(this->vProtocol.pTxProtocol);
            vSeaskyTxBuff=QByteArray(reinterpret_cast<const char*>(this->vProtocol.pTxProtocol->message_st.pData),
                        this->vProtocol.pTxProtocol->message_st.data_len);
            emit vSerialTx(vSeaskyTxBuff);
        }
    });

    vQHBoxLayout1->setContentsMargins(0,0,0,0);
    vQHBoxLayout1->setSpacing(8);
    vQHBoxLayout1->setMargin(8);
    vQHBoxLayout1->addWidget(vQCheckBox);
    vQHBoxLayout1->addWidget(vQPushButton);
    vQWidget1->setLayout(vQHBoxLayout1);


    vlineEdit1->setReadOnly(true);
    vlineEdit1->setText(tr(u8"此处QCheckBox对于定时发送有效，QPushButton对于单次发送有效，对应波形显示界面类似！"));
    vQHBoxLayout->addWidget(vQWidget1);
    vQHBoxLayout->addWidget(vlineEdit1);
    vQWidget->setLayout(vQHBoxLayout);
    vQVBoxLayout->addWidget(vQWidget);

    for(qint16 i=0;i<this->vTxNum;i++)
    {
        vQWidget = new QWidget(this->vTxEdit);
        vQHBoxLayout = new QHBoxLayout(vQWidget);
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
        connect(vlineEdit1,&QLineEdit::textChanged,[=]()
        {
            this->vTxSeasky.vName[i] = vlineEdit1->text();
            emit vInfoChanged();
        });
        connect(vlineEdit2,&QLineEdit::textChanged,[=]()
        {
            this->vTxSeasky.vQString[i] = vlineEdit2->text();
            this->vTxSeasky.vFloat[i] = this->vTxSeasky.vQString[i].toDouble();
            emit vInfoChanged();
        });
        connect(vlineEdit3,&QLineEdit::textChanged,[=]()
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

void vSeaskyPort::configQWidgetEditTx(QWidget *pQWidget,qint32 numPort,qint32 num)
{
    //    /*多LineEdit配置*/
    vTxEditScope = pQWidget;
    this->MultPleMaxCnt = num;
    vLineEditMap = new QMap<qint32,bool*>;
    vLineEditTxEnable = new bool[this->MultPleMaxCnt];
    vTxSeaskyLine = new vSeaskyData[this->MultPleMaxCnt];
    QVBoxLayout     * vQVBoxLayout = new QVBoxLayout(vTxEditScope);
    QLineEdit       * vQlineEdit;
    QLineEdit       * vQLabel;
    QLineEdit       * vEquipmentTypeEdit;  //设备类型
    QLineEdit       * vEquipmentIdEdit;    //设备ID
    QLineEdit       * vDataIdEdit;         //数据ID
    QLineEdit       * vDataLenEdit;        //数据长度
    QLineEdit       * vEquipmentTypeLabel; //设备类型
    QLineEdit       * vEquipmentIdLabel;   //设备ID
    QLineEdit       * vDataIdLabel;        //数据ID
    QLineEdit       * vDataLenLabel;       //数据长度
    QCheckBox       * vQCheckBox;
    QPushButton     * vQPushButton;
    QWidget         * vQWidgetCtr;
    QHBoxLayout     * vQHBoxLayoutCtr;
    QWidget         * vQWidget;
    QHBoxLayout     * vQHBoxLayout;

   init_protocol(pTxProtocolLine,numPort);

    vQVBoxLayout->setAlignment(Qt::AlignTop);
    vQVBoxLayout->setContentsMargins(0,0,0,0);
    vQVBoxLayout->setMargin(0);
    vQVBoxLayout->setSpacing(0);

    for(uint8_t Qv_I = 0;Qv_I<this->MultPleMaxCnt;Qv_I++)
    {
        vQWidget     = new QWidget(vTxEditScope);
        vQHBoxLayout = new QHBoxLayout(vTxEditScope);
        vQWidgetCtr  = new QWidget(vTxEditScope);
        vQHBoxLayoutCtr = new QHBoxLayout(vTxEditScope);
        //设置样式
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vQHBoxLayout->setMargin(0);
        vQHBoxLayout->setSpacing(0);
        vQHBoxLayout->setAlignment(Qt::AlignLeft);

        vQCheckBox = new QCheckBox(vTxEditScope);
        vQPushButton = new QPushButton(vTxEditScope);
        vQCheckBox->setContentsMargins(0,0,0,0);

        vEquipmentTypeLabel  = new QLineEdit(vTxEditScope);
        vEquipmentIdLabel    = new QLineEdit(vTxEditScope);
        vDataIdLabel         = new QLineEdit(vTxEditScope);
        vDataLenLabel        = new QLineEdit(vTxEditScope);

        vEquipmentTypeLabel->setMinimumSize(84,32);
        vEquipmentTypeLabel->setReadOnly(true);
        vEquipmentTypeLabel->setAlignment(Qt::AlignLeft);
        vEquipmentTypeLabel->setText("Equ_Type");

        vEquipmentIdLabel->setMinimumSize(72,32);
        vEquipmentIdLabel->setReadOnly(true);
        vEquipmentIdLabel->setAlignment(Qt::AlignLeft);
        vEquipmentIdLabel->setText("Equ_Id");

        vDataIdLabel->setMinimumSize(72,32);
        vDataIdLabel->setReadOnly(true);
        vDataIdLabel->setAlignment(Qt::AlignLeft);
        vDataIdLabel->setText("DataId");

        vDataLenLabel->setMinimumSize(72,32);
        vDataLenLabel->setReadOnly(true);
        vDataLenLabel->setAlignment(Qt::AlignLeft);
        vDataLenLabel->setText("DataLen");


        vEquipmentTypeEdit  = new QLineEdit(vTxEditScope);
        vEquipmentIdEdit    = new QLineEdit(vTxEditScope);
        vDataIdEdit         = new QLineEdit(vTxEditScope);
        vDataLenEdit        = new QLineEdit(vTxEditScope);

        vEquipmentTypeEdit->setMinimumSize(128,32);
        vEquipmentTypeEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");
        vEquipmentIdEdit->setMinimumSize(128,32);
        vEquipmentIdEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");
        vDataIdEdit->setMinimumSize(128,32);
        vDataIdEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");
        vDataLenEdit->setMinimumSize(128,32);
        vDataLenEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");

        QRegExp regx1("([0][Xx][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9])");
        QValidator *validator =
                new QRegExpValidator(regx1, this);
        vEquipmentTypeEdit->setValidator(validator);
        vEquipmentTypeEdit->setText("0X0001");
        vEquipmentIdEdit->setValidator(validator);
        vEquipmentIdEdit->setText("0X0001");
        vDataIdEdit->setValidator(validator);
        vDataIdEdit->setText("0X0001");

        QRegExp vtxFloat1("^(-?[0]|-?[1-9][0-9]{1,6})(?:\\.\\d{1,6})?$|(^\\t?$)");
        vDataLenEdit->setValidator(new QRegExpValidator(vtxFloat1,vDataLenEdit));
        vDataLenEdit->setText("2");
        //添加元素
        vQHBoxLayoutCtr->addWidget(vQCheckBox);
        vQHBoxLayoutCtr->addWidget(vQPushButton);
        vQWidgetCtr->setLayout(vQHBoxLayoutCtr);
        vQHBoxLayout->addWidget(vQWidgetCtr);
        vQHBoxLayout->addWidget(vEquipmentTypeLabel);
        vQHBoxLayout->addWidget(vEquipmentTypeEdit);
        vQHBoxLayout->addWidget(vEquipmentIdLabel);
        vQHBoxLayout->addWidget(vEquipmentIdEdit);
        vQHBoxLayout->addWidget(vDataIdLabel);
        vQHBoxLayout->addWidget(vDataIdEdit);
        vQHBoxLayout->addWidget(vDataLenLabel);
        vQHBoxLayout->addWidget(vDataLenEdit);

        connect(this,&vSeaskyPort::vQWidgetTxLineShow1,[=]()
        {
            vEquipmentTypeEdit->setText("0X"+QString::number(vTxSeaskyLine[Qv_I].vEquipmentType,
                                                          16).toUpper().rightJustified(4, QChar('0')));
            vEquipmentIdEdit->setText("0X"+QString::number(vTxSeaskyLine[Qv_I].vEquipmentId,
                                                          16).toUpper().rightJustified(4, QChar('0')));
            vDataIdEdit->setText("0X"+QString::number(vTxSeaskyLine[Qv_I].vDataId,
                                                          16).toUpper().rightJustified(4, QChar('0')));
            vDataLenEdit->setText(QString::number(vTxSeaskyLine[Qv_I].vDataLen,10));
        });
        vTxSeaskyLine[Qv_I].vFloat = new float[24];
        vTxSeaskyLine[Qv_I].vUnit = new QString[24];
        vTxSeaskyLine[Qv_I].vName = new QString[24];
        vTxSeaskyLine[Qv_I].vQString = new QString[24];

        for(uint8_t Qh_I = 0;Qh_I<24;Qh_I++)
        {
            vQLabel     = new QLineEdit(vTxEditScope);
            vQlineEdit  = new QLineEdit(vTxEditScope);
            vQLabel->setMinimumSize(60,32);
            vQLabel->setReadOnly(true);
            vQLabel->setAlignment(Qt::AlignCenter);
            vQLabel->setText(QString("%1").arg(Qv_I,2,10,QLatin1Char('0'))+":"+QString("%1").arg(Qh_I,2,10,QLatin1Char('0')));
            vQlineEdit->setMinimumSize(128,32);
            vQlineEdit->setStyleSheet("background-color:#FFFFFF;"
                                      "color:#FF4500;");
            QRegExp vtxFloat("^(-?[0]|-?[1-9][0-9]{1,6})(?:\\.\\d{1,6})?$|(^\\t?$)");
            vQlineEdit->setValidator(new QRegExpValidator(vtxFloat,vQlineEdit));
            vQlineEdit->setText("0");

            vTxSeaskyLine[Qv_I].vFloat[Qh_I] = vQlineEdit->text().toDouble();
            connect(vQlineEdit,&QLineEdit::textChanged,[=]()
            {
                vTxSeaskyLine[Qv_I].vFloat[Qh_I] = vQlineEdit->text().toDouble();
            });
            connect(this,&vSeaskyPort::vQWidgetTxLineShow2,[=]()
            {
                vQlineEdit->setText(QString("%1").arg(vTxSeaskyLine[Qv_I].vFloat[Qh_I]));
            });
            vQHBoxLayout->addWidget(vQLabel);
            vQHBoxLayout->addWidget(vQlineEdit);
        }
        vQWidget->setLayout(vQHBoxLayout);
        vQWidget->setAutoFillBackground(true);
        vQVBoxLayout->addWidget(vQWidget);
        {
            QByteArray addr;
            QString str= vEquipmentTypeEdit->text();
            if(str.length()==6)
            {
                addr=QByteArray::fromHex(
                        vEquipmentTypeEdit->text().mid(2,6).toUtf8());
                vTxSeaskyLine[Qv_I].vEquipmentType =
                        ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
            }
        }
        connect(vEquipmentTypeEdit,&QLineEdit::textChanged,[=](){
            {
                QByteArray addr;
                QString str= vEquipmentTypeEdit->text();
                if(str.length()==6)
                {
                    addr=QByteArray::fromHex(
                            vEquipmentTypeEdit->text().mid(2,6).toUtf8());
                    vTxSeaskyLine[Qv_I].vEquipmentType =
                            ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
                }
            }
        });
        {
            QByteArray addr;
            QString str= vEquipmentIdEdit->text();
            if(str.length()==6)
            {
                addr=QByteArray::fromHex(
                        vEquipmentIdEdit->text().mid(2,6).toUtf8());
                vTxSeaskyLine[Qv_I].vEquipmentId =
                        ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
            }
        }
        connect(vEquipmentIdEdit,&QLineEdit::textChanged,[=](){
            {
                QByteArray addr;
                QString str= vEquipmentIdEdit->text();
                if(str.length()==6)
                {
                    addr=QByteArray::fromHex(
                            vEquipmentIdEdit->text().mid(2,6).toUtf8());
                    vTxSeaskyLine[Qv_I].vEquipmentId =
                            ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
                }
            }
        });
        {
            QByteArray addr;
            QString str= vDataIdEdit->text();
            if(str.length()==6)
            {
                addr=QByteArray::fromHex(
                        vDataIdEdit->text().mid(2,6).toUtf8());
                vTxSeaskyLine[Qv_I].vDataId =
                        ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
            }
        }
        connect(vDataIdEdit,&QLineEdit::textChanged,[=](){
            {
                QByteArray addr;
                QString str= vDataIdEdit->text();
                if(str.length()==6)
                {
                    addr=QByteArray::fromHex(
                            vDataIdEdit->text().mid(2,6).toUtf8());
                    vTxSeaskyLine[Qv_I].vDataId =
                            ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
                }
            }
        });
        {
            QString str=vDataLenEdit->text();
            if(str!=nullptr)
            {
                //需要加入长度限制
                if(vDataLenEdit->text().toUInt()>24)
                {
                    vDataLenEdit->setText(QString("%1").arg(24));
                }
                vTxSeaskyLine[Qv_I].vDataLen
                     = vDataLenEdit->text().toUInt();
            }
        }
        connect(vDataLenEdit,&QLineEdit::textChanged,[=](){
            {
                QString str=vDataLenEdit->text();
                if(str!=nullptr)
                {
                    //需要加入长度限制
                    if(vDataLenEdit->text().toUInt()>24)
                    {
                        vDataLenEdit->setText(QString("%1").arg(24));
                    }
                    vTxSeaskyLine[Qv_I].vDataLen
                         = vDataLenEdit->text().toUInt();
                }
            }
        });
        {
            vLineEditTxEnable[Qv_I] = vQCheckBox->isChecked();
        }
        connect(vQCheckBox,&QCheckBox::released,[=](){
            vLineEditTxEnable[Qv_I] = vQCheckBox->isChecked();
        });
        connect(vQPushButton,&QPushButton::released,[=]()
        {
            //单次发送
            vDataAutoTxLines(Qv_I);
        });
        //数据绑定
        vLineEditMap->insert(Qv_I+1,&vLineEditTxEnable[Qv_I]);

    }


    vTxEditScope->setLayout(vQVBoxLayout);
    vTxEditScope->setAutoFillBackground(true);
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
                    //获取接收数据的时间戳
                    QString timeString;
                    timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                    /*加入显示*/
                    vUpdateShowBuff(timeString);
                    /*加入示波器*/
                    this->vRxdata.clear();
                    for(qint8 i=0;i<this->vProtocol.pRxProtocol->frame_st.frame_user.cmd_data.data_len;i++)
                    {
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
    this->vRxSeasky.vEquipmentType      = this->vProtocol.pRxProtocol->frame_st.frame_user.equipment_type;
    this->vRxSeasky.vEquipmentId        = this->vProtocol.pRxProtocol->frame_st.frame_user.equipment_id;
    this->vRxSeasky.vDataId        = this->vProtocol.pRxProtocol->frame_st.frame_user.data_id;
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
        if(vDataAutoTxEnable==true)
        {
            this->vProtocol.pTxProtocol->frame_st.frame_user.equipment_type = this->vTxSeasky.vEquipmentType;
            this->vProtocol.pTxProtocol->frame_st.frame_user.equipment_id = this->vTxSeasky.vEquipmentId;
            this->vProtocol.pTxProtocol->frame_st.frame_user.data_id = this->vTxSeasky.vDataId;
            this->vProtocol.pTxProtocol->frame_st.frame_user.cmd_data.data_len = this->vTxSeasky.vDataLen;
            make_protocol(this->vProtocol.pTxProtocol);
            vSeaskyTxBuff=QByteArray(reinterpret_cast<const char*>(this->vProtocol.pTxProtocol->message_st.pData),
                        this->vProtocol.pTxProtocol->message_st.data_len);
            emit vSerialTx(vSeaskyTxBuff);
        }
        else
        {
            if(vLineEditTxCtr())
            {
                vDataAutoTxLines(currIndex);
            }
        }
    }
}
//周期性调用,切换需要发送的数据,多次发送
bool vSeaskyPort::vLineEditTxCtr(void)
{
    static bool lastFindPos = 0;
    if(this->MultPleMaxCnt!=0)
    {
        QMap<qint32,bool*>::iterator findTx;
        findTx = vLineEditMap->find(this->timerCnt+1);
        if(findTx==vLineEditMap->end())
        {
            this->timerCnt = 0;
            return false;
        }
        while(!(*findTx.value()))//直到找到一个可以发送的
        {
            this->timerCnt++;
            if(this->timerCnt>=(this->MultPleMaxCnt))
            {
                //如果第一次查询直到最后也没有查询到，那么在此需要查询2次，定位到环形列表
                if(lastFindPos==0)
                {
                    lastFindPos = 1;
                    this->timerCnt = 0;
                    return(vLineEditTxCtr());
                }
                else
                {
                    //重新从0开始查询，仍然无值，说明整个列表无值，返回false
                    lastFindPos = 0;
                    this->timerCnt = 0;
                    return false;
                }
            }
            findTx = vLineEditMap->find(this->timerCnt+1);
            if(findTx==vLineEditMap->end())
            {
                this->timerCnt = 0;
                return false;
            }
        }
        currIndex = this->timerCnt;
        this->timerCnt++;
        if(this->timerCnt>=(this->MultPleMaxCnt))
        {
            this->timerCnt = 0;
            return false;
        }
        //查询到了，清除标志
        lastFindPos = 0;
        return true;
    }
    return false;
}
void vSeaskyPort::vDataAutoTxLines(uint8_t index)
{
    if(index<this->MultPleMaxCnt)
    {
        pTxProtocolLine->frame_st.frame_user.equipment_type = this->vTxSeaskyLine[index].vEquipmentType;
        pTxProtocolLine->frame_st.frame_user.equipment_id = this->vTxSeaskyLine[index].vEquipmentId;
        pTxProtocolLine->frame_st.frame_user.data_id = this->vTxSeaskyLine[index].vDataId;
        pTxProtocolLine->frame_st.frame_user.cmd_data.data_len = this->vTxSeaskyLine[index].vDataLen;
        for(uint8_t i=0;i<this->vTxSeaskyLine[index].vDataLen;i++)
        {
            pTxProtocolLine->frame_st.frame_user.cmd_data.pData[i] = ((uint32_t*)this->vTxSeaskyLine[index].vFloat)[i];
        }
        make_protocol(pTxProtocolLine);
        vSeaskyTxBuff=QByteArray(reinterpret_cast<const char*>(pTxProtocolLine->message_st.pData),
                    pTxProtocolLine->message_st.data_len);
        emit vSerialTx(vSeaskyTxBuff);
    }


}
vSeaskyPortQThread::vSeaskyPortQThread(QObject *parent) : QThread(parent)
{

}
