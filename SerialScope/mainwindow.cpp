#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSpinBox>
#include <openglscene.h>
#include <QApplication>
#include <QObject>
#include <QGLWidget>
#include <QGraphicsView>
#include <QResizeEvent>
#include <vopenglwidget.h>
#include <QFileDialog>
#include <QDebug>

#ifdef WinDownVersion
#pragma comment(lib,"user32.lib")
#endif

/*-----------界面刷新相关的定时器-----------*/
const qint32 vTimerRxPlainText   = 100;
const qint32 vTimerRxPlainSeasky = 100;
const qint32 vTimerRxSeasky      = 100;
const qint32 vTimerRxScope       = 75;

const qint16  SerialMutipSendNum = 200;//串口调试助手多条发送模式条目数量
const qint32  SerialTxTimerCfg   = 100;//默认串口发送周期，可以通过spinBox调整
const qint32  SeaskyPortNum      = 24; //最大支持的FLOAT数据长度
const qint32  Utf8MaxLen         = 10+SeaskyPortNum*4;
const qint32  SeaskyTimer        = 100;//协议窗口数据刷新频率
const QString FindSerialCommand  = "SerialCommand";
/*Seaky 协议使用相关数据*/
QString vRxQString[SeaskyPortNum];
QString vRxName[SeaskyPortNum];
QString vRxUnit[SeaskyPortNum];
float   vRxfloat[SeaskyPortNum];
uint8_t vRxUtf8[Utf8MaxLen];
QString vTxQString[SeaskyPortNum];
QString vTxName[SeaskyPortNum];
QString vTxUnit[SeaskyPortNum];
float   vTxfloat[SeaskyPortNum];
uint8_t vTxUtf8[Utf8MaxLen];
bool    rxSeaskyHexEnable = false;       //16进制格式接收使能

const QString ModulePath = "/Config/ModulePath";
const QString CfgPath    = "/Config";
class GraphicsView : public QGraphicsView
{
public:
    GraphicsView()
    {
        this->setWindowFlags(Qt::CustomizeWindowHint);
    }
protected:
    void resizeEvent(QResizeEvent *event) {
        if (scene())
            scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
        QGraphicsView::resizeEvent(event);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    //0.初始化相关的地址
    vDependenceAddr();
    //1.加载自定义图形
    vShowInit();
    //2.串口检测相关的初始化,检测串口设备
    vInitDetection();
    //3.串口设置内容初始化，波特率等
    vUpdateComInfo();
    //4.初始化串口设备更新支持
    vUpdateSerial();
    //5.加载信息发送配置
    vInitSerialTx();
    //6.加载信息接收配置
    vInitSerialRx();
    //7.串口信息更新
    vInfoChangedInit();
    //8.串口协议初始化
    vInitSeasky();
    //9.加载控件列表->8个按钮->目前实现一个->后期扩展对应功能->数量方便改变
    vInitControl();//设置控件
    //10.配置刷新相关的定时器
    vShowTimerCfg();
    //
    vReadSettings();
    /*更新一次界面显示*/
    vUpdateShow();
    //保存数据
    connect(ui->pushButtonRx1,&QPushButton::released,
            this,&MainWindow::vSaveRxText);
    connect(ui->pushButton_2,&QPushButton::released,
            this,&MainWindow::vSaveRxSeaskyText);
    vStatusbarCfg();
    qDebug()<<"main tid:MainWindow"<< QThread::currentThreadId();
}
//析构函数
MainWindow::~MainWindow()
{
    ui->plainTextRx->TimerStop();
    ui->plainTextEdit->TimerStop();
    this->vSerialCtr.vSeaskyPortCtr.timerStop();
    ui->widgetScope->TimerStop();

    this->vSerialCtr.vSeaskyPortCtr.vQTimerTxStop();
    this->vSerialCtr.vQObjectTxCtr.vTimerStop();

    vRxTxInfoTimer.stop();
    delete ui;
}
//退出事件->读取存储配置
void MainWindow::closeEvent(QCloseEvent *event)
{
    vWriteSettings();
    event->accept();
}

//0. 配置相关地址，应包含所有地址初始化操作
void MainWindow::vDependenceAddr(void)
{
    /*Seaky 协议使用相关数据*/
    //设置依赖的两个Widget 16进制显示窗口
    this->vSerialCtr.vSeaskyPortCtr.setQWidgetAddr(
                ui->scrollAreaWidgetContents_2,
                ui->scrollAreaWidgetContents_3);
    this->vSerialCtr.vSeaskyPortCtr.setRxSeaskyAddr(
                &vRxQString[0],&vRxName[0],&vRxUnit[0],&vRxfloat[0]);
    this->vSerialCtr.vSeaskyPortCtr.setTxSeaskyAddr(
                &vTxQString[0],&vTxName[0],&vTxUnit[0],&vTxfloat[0]);
    /*波形显示控件波形名称查询地址*/
    ui->widgetScope->vSetNameAddr(&vRxName[0]);
    /*协议操作地址受此分配*/
    this->vSerialCtr.vSeaskyPortCtr.vProtocol.rx_info.utf8_data = &vRxUtf8[0];
    this->vSerialCtr.vSeaskyPortCtr.vProtocol.rx_info.data =     &vRxfloat[0];
    this->vSerialCtr.vSeaskyPortCtr.vProtocol.tx_info.utf8_data = &vTxUtf8[0];
    this->vSerialCtr.vSeaskyPortCtr.vProtocol.tx_info.data =     &vTxfloat[0];
    /*显示数据地址*/
    ui->plainTextRx->SetShowBuffAddr(&this->vSerialCtr.vSerial.vSerialData->RxBuff);
    //串口发送，hex格式共享，所有控件建议只读
    ui->plainTextTx->setHexEnableAddr(
                &this->vSerialCtr.vSerial.vSerialData->txHexEnable);
    //串口接收，hex格式共享，所有控件建议只读
    ui->plainTextRx->setHexEnableAddr(
                &this->vSerialCtr.vSerial.vSerialData->rxHexEnable);
    ui->plainTextEdit->setHexEnableAddr(&rxSeaskyHexEnable);
    //设置发送相关地址
    this->vSerialCtr.vQObjectTxCtr.vTxAddrSet(&ui->plainTextTx->TextTxBuff);

}
//1.0 加载自定义图形，主要是需要程序重复添加的图形
void MainWindow::vShowInit(void)
{
    ui->statusbar->setSizeGripEnabled(false);//去掉状态栏右下角的三角
    //创建多个Rx\Tx显示窗口
    this->vSerialCtr.vSeaskyPortCtr.configQWidgetRx(
                SeaskyPortNum);
    this->vSerialCtr.vSeaskyPortCtr.configQWidgetTx(
                SeaskyPortNum);
    //初始化多条信息发送窗口
    vLineEditShowInit(SerialMutipSendNum);
    //关于窗口MAC信息
    ui->textBrowser->setInfo(vCafes.get_localmachine_name(),
                             vCafes.get_localmachine_mac());

    connect(ui->tabWidget1,&QTabWidget::currentChanged,
            this,&MainWindow::vTabTimerCfg);

}
//1.1
void MainWindow::vLineEditShowInit(qint32 MultPleNum)
{
    /*多LineEdit配置*/
    this->MultPleMaxCnt = MultPleNum;
    QVBoxLayout     * vQVBoxLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
    QPushButton     * vpushButton,
                    * vPathButton;
    vQLineEditHex   * vlineEdit;
    QCheckBox       * vcheckBox;
    QLineEdit       * vQlineEdit;
    //分配依赖的地址
    LineEditData = new QByteArray[MultPleMaxCnt];
    LineEditMap =  new QMap<qint32,bool*>;
    //原始数据地址共享,该地址只能在此分配
    this->vSerialCtr.vQObjectTxCtr.vAddrSet(
                &this->MultPleMaxCnt,
                &LineEditData[0],
                LineEditMap);
    //添加一个自动导入命令的操作支持
    /*-------------------导入命令配置-------------------*/
    QWidget     * vQWidget     = new QWidget(ui->scrollAreaWidgetContents);
    QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);
    vQHBoxLayout->setContentsMargins(0,0,0,0);
    vPathButton   = new QPushButton(vQWidget);
    vQlineEdit    = new QLineEdit(vQWidget);
    vpushButton   = new QPushButton(QString::fromLocal8Bit("导入"),vQWidget);
    vPathButton->setText("./");
    vPathButton->setStyleSheet("color:#FFFFFF;font-weight: bold;");
    vPathButton->setMinimumSize(24,24);
    vPathButton->setMaximumSize(24,24);
    vQlineEdit->setMinimumSize(0,24);
    vQlineEdit->setMaximumSize(16777215,24);
    vQlineEdit->setStyleSheet("background-color:#FFFFFF;"
                              "color:#0099FF;");
    vpushButton->setMinimumSize(48,24);
    vpushButton->setMaximumSize(48,24);
    vQHBoxLayout->addWidget(vPathButton);
    vQHBoxLayout->addWidget(vQlineEdit);
    vQHBoxLayout->addWidget(vpushButton);
    vQWidget->setLayout(vQHBoxLayout);
    vQVBoxLayout->addWidget(vQWidget);
    connect(vPathButton,&QPushButton::released,[=]()
    {
        //获取文件名称，和路径
        QString dirpath =
                QFileDialog::getOpenFileName(this,
                                             QStringLiteral("获取文件配置路径"),
                                             qApp->applicationDirPath(), QString(tr("File (*.csv)")));
        if(dirpath!=NULL)
        {
            vQlineEdit->setText(dirpath);
        }
    });
    //
    connect(vpushButton,&QPushButton::released,[=]()
    {
        vImportLineText(vQlineEdit->text());
    });
    /*-------------------导入命令配置-------------------*/
    for(qint16 i=0;i<this->MultPleMaxCnt;i++)
    {
        QWidget     * vQWidget     = new QWidget(ui->scrollAreaWidgetContents);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vcheckBox   = new QCheckBox(vQWidget);
        vlineEdit   = new vQLineEditHex(vQWidget);
        vpushButton = new QPushButton(QString::number(i+1,10),vQWidget);

        vcheckBox->setMinimumSize(24,24);
        vcheckBox->setMaximumSize(24,24);
        vlineEdit->setMinimumSize(0,24);
        vlineEdit->setMaximumSize(16777215,24);
        vlineEdit->setStyleSheet("background-color:#FFFFFF;"
                                 "color:#0099FF;");
        //设置窗口ID
        vlineEdit->QLineId = i+1;
        //初始化是否被选中
        vlineEdit->isTxEnable =
                vcheckBox->isChecked();
        //分配地址
        vlineEdit->setTextTxBuffAddr(&LineEditData[i]);
        //该地址是当前发送配置地址，可以更改
        vpushButton->setMinimumSize(48,24);
        vpushButton->setMaximumSize(48,24);
        vQHBoxLayout->addWidget(vcheckBox);
        vQHBoxLayout->addWidget(vlineEdit);
        vQHBoxLayout->addWidget(vpushButton);
        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);
        //hex格式共享，所有控件建议只读
        vlineEdit->setHexEnableAddr(&this->vSerialCtr.vSerial.vSerialData->txHexEnable);
        /*发送端hex配置,本地如果hex格式标志改变，使用信号与槽更新每个控件的显示*/
        connect(this,
                &MainWindow::txHexEnableChanged,
                vlineEdit,
                &vQLineEditHex::hexEnableChanged);

        //绑定信号与槽，更新是否使能发送
        connect(vcheckBox,&QCheckBox::released,[=]()
        {
            vlineEdit->isTxEnable = vcheckBox->isChecked();
        });
        //由于多条发送，此处需要传递需要发送的数据的ID
        connect(vpushButton,&QPushButton::released,[=]()
        {
            emit lineEditTxOne(vlineEdit->QLineId-1);
        });
        //map查表
        LineEditMap->insert(i+1,&vlineEdit->isTxEnable);
    }
    ui->scrollAreaWidgetContents->setLayout(vQVBoxLayout);
    //触发LineEdit发送
    connect(this,&MainWindow::lineEditTxOne,
            &this->vSerialCtr.vQObjectTxCtr,
            &vQObjectTx::vLineEditTxOne,
            Qt::QueuedConnection);
}
/******************************************************************************************************/
/**********************************************串口插拔检测**********************************************/
/******************************************************************************************************/
//2.0 串口设备初始化操作
void MainWindow::vInitDetection(void)
{
    //USB插拔事件   Windown 版本
#ifdef WinDownVersion
    vWindownInit();
    //打开串口
    connect(ui->pushButtonOpen,&QPushButton::released,this,&MainWindow::SerialOpen);

#else

#endif
    //打开串口时阻塞主线程，等待返回是否打开成功
    connect(this,&MainWindow::vOpenSerial,
            &this->vSerialCtr.vSerial,
            &vSerialPort::vSerialOpen,
            Qt::BlockingQueuedConnection);
    connect(this,&MainWindow::vCloseSerial,
            &this->vSerialCtr.vSerial,
            &vSerialPort::vSerialClose,
            Qt::QueuedConnection);

    connect(&this->vSerialCtr.vSerial,&vSerialPort::vTxError,
            this,&MainWindow::vTxError);
}
#ifdef WinDownVersion
/*************此部分代码调用了windows控件，不适用于Linux，如需移植，可以尝试替换************************/
//注册插拔事件,调用了Windown控件
//2.1 串口设备初始化操作
void MainWindow::vWindownInit(void)
{
    static const GUID GUID_DEVINTERFACE_LIST[] =
    {
    // GUID_DEVINTERFACE_USB_DEVICE
    {0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
//    // GUID_DEVINTERFACE_DISK
//    { 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
//    // GUID_DEVINTERFACE_HID,
//    { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
//    // GUID_NDIS_LAN_CLASS
//    { 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } },
//    // GUID_DEVINTERFACE_COMPORT
//    { 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },
//    // GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
//    { 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },
//    // GUID_DEVINTERFACE_PARALLEL
//    { 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },
//    // GUID_DEVINTERFACE_PARCLASS
//    { 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } },
    };
    //注册插拔事件
    HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotifacationFiler;
    ZeroMemory(&NotifacationFiler,sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    NotifacationFiler.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotifacationFiler.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    for(int i=0;i<sizeof(GUID_DEVINTERFACE_LIST)/sizeof(GUID);i++)
    {
        NotifacationFiler.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
        //GetCurrentUSBGUID();
        hDevNotify = RegisterDeviceNotification(HANDLE(this->winId()),&NotifacationFiler,DEVICE_NOTIFY_WINDOW_HANDLE);
        if(!hDevNotify)
        {
            GetLastError();
        }
    }
}
/*USB设备插入触发事件*/
//2.2 串口设备依赖
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);//第一层解算
    UINT msgType = msg->message;
    if(msgType==WM_DEVICECHANGE)
    {
        PDEV_BROADCAST_HDR lpdb = PDEV_BROADCAST_HDR(msg->lParam);//第二层解算
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE pDevInf  = PDEV_BROADCAST_DEVICEINTERFACE(lpdb);
//                QString device_name = QString::fromLocal8Bit("插入设备(name)：")+QString::fromWCharArray(pDevInf->dbcc_name,int(pDevInf->dbcc_size)).toUtf8();
//                qDebug()<< device_name;
                //插入设备重新扫描
                vUpdateSerial();
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE pDevInf  = PDEV_BROADCAST_DEVICEINTERFACE(lpdb);
//                QString device_name = QString::fromLocal8Bit("移除设备(name):")+QString::fromWCharArray(pDevInf->dbcc_name,int(pDevInf->dbcc_size)).toUtf8();
//                qDebug()<< device_name;
                //移除设备重新扫描
                vUpdateSerial();
            }
            break;
        }
    }
    return false;
}

#else
//此处添加LINUX替代函数支持



/*************此部分代码调用了windows控件，不适用于Linux，如需移植，可以尝试替换************************/
/*扫描串口设备，检测设定串口是否还存在，如果已打开串口消失，自定定位到其他COM口，需要手动开启串口*/
#endif
/******************************************************************************************************/
/**********************************************串口插拔检测**********************************************/
/******************************************************************************************************/

//3.串口设置内容初始化，波特率等
void MainWindow::vUpdateComInfo(void)
{

    /*comboBoxCom1相关字符需要从设备读取*/

    /*comboBox配置波特率*/
//    ui->comboBoxCom2->addItem("4000000",int(4000000));
//    ui->comboBoxCom2->addItem("3500000",int(3500000));
//    ui->comboBoxCom2->addItem("3000000",int(3000000));
//    ui->comboBoxCom2->addItem("2500000",int(2500000));
    /*CP2104仅支持2M波特率*/
    ui->comboBoxCom2->addItem("2000000",int(2000000));
    ui->comboBoxCom2->addItem("1382400",int(1382400));
    ui->comboBoxCom2->addItem("921600",int(921600));
    ui->comboBoxCom2->addItem("460800",int(460800));
    ui->comboBoxCom2->addItem("256000",int(256000));
    ui->comboBoxCom2->addItem("230400",int(230400));
    ui->comboBoxCom2->addItem("128000",int(128000));
    ui->comboBoxCom2->addItem("115200",int(115200));
    ui->comboBoxCom2->addItem("76800",int(76800));
    ui->comboBoxCom2->addItem("57600",int(57600));
    ui->comboBoxCom2->addItem("43000",int(43000));
    ui->comboBoxCom2->addItem("38400",int(38400));
    ui->comboBoxCom2->addItem("19200",int(19200));
    ui->comboBoxCom2->addItem("14400",int(14400));
    ui->comboBoxCom2->addItem("9600",int(9600));
    ui->comboBoxCom2->addItem("4800",int(4800));
    ui->comboBoxCom2->addItem("2400",int(2400));
    ui->comboBoxCom2->addItem("1200",int(1200));
    //默认选择115200
    ui->comboBoxCom2->setCurrentText("115200");

    /*comboBox配置停止位*/
    ui->comboBoxCom3->addItem("1",QSerialPort::StopBits(QSerialPort::OneStop));
    ui->comboBoxCom3->addItem("1.5",QSerialPort::StopBits(QSerialPort::OneAndHalfStop));
    ui->comboBoxCom3->addItem("2",QSerialPort::StopBits(QSerialPort::TwoStop));

    /*comboBox配置数据位*/
    ui->comboBoxCom4->addItem("8",QSerialPort::DataBits(QSerialPort::Data8));
    ui->comboBoxCom4->addItem("7",QSerialPort::DataBits(QSerialPort::Data7));
    ui->comboBoxCom4->addItem("6",QSerialPort::DataBits(QSerialPort::Data6));
    ui->comboBoxCom4->addItem("5",QSerialPort::DataBits(QSerialPort::Data5));

    /*comboBox配置奇偶校验*/
    ui->comboBoxCom5->addItem(QString::fromLocal8Bit("无"),QSerialPort::Parity(QSerialPort::NoParity));
    ui->comboBoxCom5->addItem(QString::fromLocal8Bit("偶校验"),QSerialPort::Parity(QSerialPort::EvenParity));
    ui->comboBoxCom5->addItem(QString::fromLocal8Bit("无奇校验"),QSerialPort::Parity(QSerialPort::OddParity));

    /*配置编码格式*/
    ui->comboBoxCom6->addItem("NoCode",QSerialPort::Parity(SerialCodeNo));
    ui->comboBoxCom6->addItem("UTF8",QSerialPort::Parity(SerialCodeUtf8));
    ui->comboBoxCom6->addItem("UTF16",QSerialPort::Parity(SerialCodeUtf16));
    ui->comboBoxCom6->addItem("GB18030",QSerialPort::Parity(SerialCodeGB18030));
    ui->comboBoxCom6->addItem("ISO8859",QSerialPort::Parity(SerialCodeISO8859));
    ui->comboBoxCom6->addItem("BIG5",QSerialPort::Parity(SerialCodeBig5));
    ui->comboBoxCom6->addItem("Shift-JIS",QSerialPort::Parity(SerialShiftJIS));

    /*设置数据流控制*/
    ui->comboBoxCom7->addItem("NoFlowControl",QSerialPort::Parity(QSerialPort::NoFlowControl));
    ui->comboBoxCom7->addItem("RTS/CTS",QSerialPort::Parity(QSerialPort::HardwareControl));
    ui->comboBoxCom7->addItem("XON/XOFF",QSerialPort::Parity(QSerialPort::SoftwareControl));
};

//4.初始化串口设备更新支持
void MainWindow::vUpdateSerial(void)
{
    bool haveIt = false;
    static qint16 index;//记录上次打开选中
    this->vSerialCtr.vSerial.vSerialComScanf();
    ui->comboBoxCom1->clear();
    haveIt = false;
    index  = 0;
    if(this->vSerialCtr.vSerial.vSerialPortCnt!=0)
    {
        for(qint16 i=0;i<this->vSerialCtr.vSerial.vSerialPortCnt;i++)
        {
            QString str ="("
                    +this->vSerialCtr.vSerial.vSerialComInfo[i].SerialName
                    +")"
                    +this->vSerialCtr.vSerial.vSerialComInfo[i].DescribeStr;
            ui->comboBoxCom1->addItem(str,
                this->vSerialCtr.vSerial.vSerialComInfo[i].SerialName);

            if(this->vSerialCtr.vSerial.vSerialComInfo[i].SerialName ==
               this->vSerialCtr.vSerial.vSerialConfig->vSerialPortName)
            {
                haveIt = true;
                index = i;
            }
        }
        //检测上次打开的串口是否存在
        if(haveIt != false)
        {
            ui->comboBoxCom1->setCurrentIndex(index);
        }
        else
        {
            //如果之前串口为打开状态，在串口拔出时，串口应变为关闭，因此按钮需要同步
            this->SerialClose();
            this->vSerialCtr.vSerial.vSerialConfig->vSerialPortName =
                    ui->comboBoxCom1->currentData().toString();
        }
    }
    else
    {
        this->SerialClose();
        ui->comboBoxCom1->clear();
        this->vSerialCtr.vSerial.vSerialConfig->vSerialPortName =" ";
    }
};

//5.加载信息发送配置
void MainWindow::vInitSerialTx(void)
{
    //首次使用，配置默认时间
    this->vSerialCtr.vQObjectTxCtr.vTimerSet(SerialTxTimerCfg);
    ui->spinBox->setValue(SerialTxTimerCfg);
    //发送的模式控制
    connect(ui->checkBox_1,
            &QCheckBox::released,
            this,
            &MainWindow::vTxModeCfg);
    /*协议定时发送相关*/
    connect(ui->pushButton_3,&QPushButton::released,
            this,
            &MainWindow::vTxModeCfg);
    connect(ui->spinBox_2,&QSpinBox::editingFinished,
            this,
            &MainWindow::vTxModeCfg);
    connect(ui->checkBox_4,&QCheckBox::released,[=]()
    {
        //更新vQObjectTxCtr的发送定时器时间
        vTxModeTimerCfg();
    });
    connect(ui->spinBox,&QSpinBox::editingFinished,[=]()
    {
        //更新vQObjectTxCtr的发送定时器时间
        vTxModeTimerCfg();
    });
    //发送hex模式切换
    connect(this,
            &MainWindow::txHexEnableChanged,
            ui->plainTextTx,
            &vQTextEdit::hexEnableChanged);
    //发送窗口hex格式改变
    connect(ui->checkBox_2,&QCheckBox::released,
            this,&MainWindow::vTxHexEnableCfg);
    //发送加入换行符
    connect(ui->checkBox_3,&QCheckBox::released,[=]()
    {
        vTxStampCfg();
    });
    //定时器发送启动控制
    connect(ui->pushButtonTx1,
            &QPushButton::released,
            &this->vSerialCtr.vQObjectTxCtr,
            &vQObjectTx::vTxTimeOut
            ,Qt::QueuedConnection);
    //根据选择内容清空对应发送数据
    connect(ui->pushButtonTx2,&QPushButton::released,[=]()
    {
        if(!ui->checkBox_1->isChecked())
        {
            ui->plainTextTx->TextTxBuff.clear();
            //更新显示
            emit txHexEnableChanged();
        }
        else
        {
            for(qint16 i=0;i<this->MultPleMaxCnt;i++)
            {
                LineEditData[i].clear();
            }
            //更新显示
            emit txHexEnableChanged();
        }
    });
}

//6.加载信息接收配置
void MainWindow::vInitSerialRx(void)
{
    //接收消息，清空处理
    connect(ui->pushButtonRx2,&QPushButton::released,
            ui->plainTextRx,&vPlainTextEdit::clearBuff);
    //切换数据接收函数
    connect(ui->pushButton_4,&QPushButton::released,
            this,&MainWindow::vRxSlotChanged);
    //刷新窗口控制
    connect(this,&MainWindow::plaintextEditShowOne,
            ui->plainTextRx,
            &vPlainTextEdit::hexEnableChanged);
    //如果遇到hex切换操作，刷新一次窗口
    connect(ui->checkBoxRx1,&QCheckBox::released,
            this,&MainWindow::vRxHexEnableCfg);
    //是否是使能了串口接收加入时间戳
    connect(ui->checkBoxRx2,&QCheckBox::released,
            this,&MainWindow::vRxTimerStampChanged);
}
//7.串口信息更新,只需要初始化一次
void MainWindow::vInfoChangedInit(void)
{
    //串口名称-仅在打开串口时更新选中
//    connect(ui->comboBoxCom1,&QComboBox::currentTextChanged,[=]()
//    {
//        this->vSerialCtr.vSerial.vSerialConfig->vSerialPortName =
//                ui->comboBoxCom1->currentData().toString();
//    });
    //串口波特率
    connect(ui->comboBoxCom2,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialBaudRate =
                ui->comboBoxCom2->currentData().toInt();
    });
    //串口数据位
    connect(ui->comboBoxCom3,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialStopBits =
                QSerialPort::StopBits(ui->comboBoxCom3->currentData().toInt());
    });
    connect(ui->comboBoxCom4,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialDataBits =
                QSerialPort::DataBits(ui->comboBoxCom4->currentData().toInt());
    });
    //串口极性
    connect(ui->comboBoxCom5,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialParrity  =
                QSerialPort::Parity(ui->comboBoxCom5->currentData().toInt());
    });
    //串口编码格式
    connect(ui->comboBoxCom6,&QComboBox::currentTextChanged,[=]()
    {
        *this->vSerialCtr.vSerial.vCodeConver =
                vSerialCodeModeEnum(ui->comboBoxCom6->currentData().toInt());
    });
    //串口流控
    connect(ui->comboBoxCom7,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialFlowControl =
                QSerialPort::FlowControl(ui->comboBoxCom7->currentData().toInt());
    });
}
//8.初始化协议配置
void MainWindow::vInitSeasky(void)
{
    //设置vSeaskyPortCtr依赖的界面刷新窗口
    this->vSerialCtr.vSeaskyPortCtr.setPlainEdit(
                ui->plainTextEdit);
    /*-------------正则表达式,限制输入----------------*/
    QRegExp regx1("([0][Xx][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9])");
    QValidator *validator =
            new QRegExpValidator(regx1, this);
    ui->lineEdit_4->setValidator(validator);
    ui->lineEdit_6->setValidator(validator);
    QValidator *IntValidator=new QIntValidator(0,99,this);
    ui->lineEdit_8->setValidator(IntValidator);
    ui->lineEdit_8->setPlaceholderText("请输入1-255");
//    ui->lineEdit_8->setValidator(validator2);
    /*-------------正则表达式,限制输入----------------*/
    //TX的CMD_ID数据更新
    connect(ui->lineEdit_4,&QLineEdit::editingFinished,[=]()
    {
        QByteArray addr;
        QString str= ui->lineEdit_4->text();
        if(str.length()==6)
        {
            addr=QByteArray::fromHex(
                    ui->lineEdit_4->text().mid(2,6).toUtf8());
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vCmdId =
                    ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
            //配置保存到配置文件
            vSaveModule();
        }
    });
    //TX的寄存器配置更新
    connect(ui->lineEdit_6,&QLineEdit::editingFinished,[=]()
    {
        QByteArray addr;
        QString str= ui->lineEdit_6->text();
        if(str.length()==6)
        {
            addr=QByteArray::fromHex(
                    ui->lineEdit_6->text().mid(2,6).toUtf8());
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vReg =
                    ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
            vSaveModule();
        }
    });
    //TX的vDataLen长度更新
    connect(ui->lineEdit_8,&QLineEdit::editingFinished,[=]()
    {
        QString str=ui->lineEdit_8->text();
        if(str!=nullptr)
        {
            //需要加入长度限制
            if(ui->lineEdit_8->text().toUInt()>SeaskyPortNum)
            {
                ui->lineEdit_8->setText(QString("%1").arg(SeaskyPortNum));
            }
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataLen
                 = ui->lineEdit_8->text().toUInt();
            vSaveModule();
        }
    });
    //数据更新，子控件通知父级刷新界面
    connect(&this->vSerialCtr.vSeaskyPortCtr,
            &vSeaskyPort::showRxHead,
            this,
            &MainWindow::showRxHead);
    //绑定图形绘制数据传输的信号与槽
    connect(&this->vSerialCtr.vSeaskyPortCtr,
            &vSeaskyPort::RxScope,
            ui->widgetScope,
            &vQCustomPlot::SeaskyScope,
            Qt::QueuedConnection);
    //绑定图形绘制数据传输的信号与槽,数据分发到OPenGl绘图
    connect(&this->vSerialCtr.vSeaskyPortCtr,
            &vSeaskyPort::RxScope,
            &this->vOpenGlWidget,
            &vOpenGlWidget::getCapeEuler,
            Qt::QueuedConnection);
    //模型改变
    void (QComboBox::*vChanged)(int)=&QComboBox::activated;
    connect(ui->comboBox,vChanged,
            this,&MainWindow::vModuleChanged);
    //数据改变，保存配置
    connect(&this->vSerialCtr.vSeaskyPortCtr,
            &vSeaskyPort::vInfoChanged,
            this,&MainWindow::vSaveModule);
    //设置发送寄存器可以编译，接收寄存器不可以编辑
    ui->widgetScope1->vSetButtonEnable(true);
    ui->widgetScope2->vSetButtonEnable(false);
    //设置按键颜色
    ui->widgetScope1->setStyleSheet("background-color:#1296db;");
    ui->widgetScope2->setStyleSheet("background-color:#d81e06;");
    //发送寄存器改变，更新数据
    connect(ui->widgetScope1,&vCustomWidget::vFlagChanged,[=]()
    {
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vReg
                = ui->widgetScope1->flag_t;
        //刷新界面
        vPortShow();
    });
    //清除数据
    connect(ui->pushButton,&QPushButton::released,
            ui->plainTextEdit,
            &vPlainTextEdit::clearBuff);
    //设置协议数据刷新定时器，并开启
    this->vSerialCtr.vSeaskyPortCtr.setTimer(SeaskyTimer);
    this->vSerialCtr.vSeaskyPortCtr.timerStart();
    //开启界面定时器刷新
    ui->widgetScope->TimerStart();
}
//9.加载控件列表->8个按钮->目前实现一个->后期扩展对应功能
void MainWindow::vInitControl(void)
{
    OpenGLScene   *vopenGl = new OpenGLScene;
    vOpenGlWidget.setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    vOpenGlWidget.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    vOpenGlWidget.setScene(vopenGl);
    QObject::connect(vopenGl,
                     &OpenGLScene::mouseMove,
                     &vOpenGlWidget,
                     &vOpenGlWidget::vmouseMoveEvent);
    QObject::connect(&vOpenGlWidget,
                     &vOpenGlWidget::loadMode,
                     vopenGl,
                     &OpenGLScene::vloadModel);
    QObject::connect(&vOpenGlWidget,
                     &vOpenGlWidget::setModelColor,
                     vopenGl,
                     &OpenGLScene::setModelColor);
    QObject::connect(&vOpenGlWidget,
                     &vOpenGlWidget::setBackgroundColor,
                     vopenGl,
                     &OpenGLScene::setBackgroundColor);
    QObject::connect(&vOpenGlWidget,
                     &vOpenGlWidget::vAxisTurnChange,
                     vopenGl,
                     &OpenGLScene::CoordinateRotation);
    QObject::connect(&vOpenGlWidget,
                     &vOpenGlWidget::ZeroAngleCfg,
                     vopenGl,
                     &OpenGLScene::ZeroAngleCfg);
    //初始界面大小
    vOpenGlWidget.resize(1024,768);
    connect(&this->vOpenGlWidget,
            &vOpenGlWidget::setCapeEuler,
            vopenGl,
            &OpenGLScene::setCapeEuler);

    QString ImagePath[8]={" "};
    ImagePath[0] =
            QString("background-image:url(:/image/image/v3d0.png);");
    ImagePath[1] =
            QString("background-image:url(:/image/image/v3d1.png);");
    ImagePath[2] =
            QString("background-image:url(:/image/image/v3d2.png);");
    ImagePath[3] =
            QString("background-image:url(:/image/image/v3d3.png);");
    QVBoxLayout   * vQVBoxLayout =
            new QVBoxLayout(ui->scrollAreaWidgetContents_4);
    QPushButton   * vQPushButton1;
    QPushButton   * vQPushButton2;
    //创建8个
    for(qint16 i=0;i<4;i++)
    {
        QWidget     * vQWidget = new QWidget(ui->scrollAreaWidgetContents_4);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);

        vQPushButton1 = new QPushButton(vQWidget);
        vQPushButton2 = new QPushButton(vQWidget);
        vQPushButton1->setMinimumSize(68,68);
        vQPushButton1->setMaximumSize(68,68);
        vQPushButton2->setMinimumSize(68,68);
        vQPushButton2->setMaximumSize(68,68);
        vQPushButton1->setStyleSheet(ImagePath[2*i]);
        vQPushButton2->setStyleSheet(ImagePath[2*i+1]);
        vQHBoxLayout->addWidget(vQPushButton1);
        vQHBoxLayout->addWidget(vQPushButton2);

        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);

        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vQHBoxLayout->setMargin(1);
        vQVBoxLayout->setMargin(1);
        //绑定信号与槽，按下触发
        connect(vQPushButton1,&QPushButton::released,[=]()
        {
            vControlCmd(2*i+1);
        });
        connect(vQPushButton1,&QPushButton::released,[=]()
        {
            vControlCmd(2*i+2);
        });
    }
}
//定时器初始化
void MainWindow::vShowTimerCfg(void)
{
    ui->plainTextRx->TimerStop();
    ui->plainTextEdit->TimerStop();
    this->vSerialCtr.vSeaskyPortCtr.timerStop();
    ui->widgetScope->TimerStop();

    ui->plainTextRx->setTimerCfg(vTimerRxPlainText);
    ui->plainTextEdit->setTimerCfg(vTimerRxPlainSeasky);
    this->vSerialCtr.vSeaskyPortCtr.setTimer(vTimerRxSeasky);
    ui->widgetScope->setQCustomPlotTimer(vTimerRxScope);

    ui->plainTextRx->TimerStart();
    ui->plainTextEdit->TimerStart();
    this->vSerialCtr.vSeaskyPortCtr.timerStart();
    ui->widgetScope->TimerStart();
    vTabTimerCfg();
}
//读取串口配置
void MainWindow::readSerialChange(void)
{
    this->vSerialCtr.vSerial.vSerialConfig->vSerialPortName = ui->comboBoxCom1->currentData().toString();
    this->vSerialCtr.vSerial.vSerialConfig->vSerialBaudRate = ui->comboBoxCom2->currentData().toInt();
    this->vSerialCtr.vSerial.vSerialConfig->vSerialStopBits = QSerialPort::StopBits(ui->comboBoxCom3->currentData().toInt());
    this->vSerialCtr.vSerial.vSerialConfig->vSerialDataBits = QSerialPort::DataBits(ui->comboBoxCom4->currentData().toInt());
    this->vSerialCtr.vSerial.vSerialConfig->vSerialParrity  = QSerialPort::Parity(ui->comboBoxCom5->currentData().toInt());
    this->vSerialCtr.vSerial.vSerialConfig->vSerialFlowControl = QSerialPort::FlowControl(ui->comboBoxCom7->currentData().toInt());
}
//打开串口设备
void MainWindow::SerialOpen(void)
{
    if(ui->pushButtonOpen->isChecked())
    {
        //读取选择的串口配置
        readSerialChange();
        bool isOpen;
        emit vOpenSerial(isOpen);
        if(isOpen)
        {
            ui->pushButtonOpen->setChecked(true);
        }
        else
        {
            //打开串口失败
            this->doWarning(QString::fromLocal8Bit("打开串口失败，请检查串口是否插入！"));
            this->SerialClose();
        }
    }
    else
    {
        this->SerialClose();
    }
}
//关闭串口设备
void MainWindow::SerialClose(void)
{
    ui->pushButtonOpen->setChecked(false);
    emit vCloseSerial();
}
//启动时读取设置
void MainWindow::vReadSettings(void)
{
    //获取配置路径
    QString path = qApp->applicationDirPath()+
            CfgPath+"/"+"config.ini";
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("Configuration");
    //恢复上次的界面位置和大小
    resize(settings.value("size",QSize(400,400)).toSize());
    move(settings.value("pos",QPoint(200,200)).toPoint());
    //串口配置
    ui->comboBoxCom2->setCurrentIndex(settings.value("PortName",QVariant(7)).toInt());
    ui->comboBoxCom3->setCurrentIndex(settings.value("BaudRate",QVariant(0)).toInt());
    ui->comboBoxCom4->setCurrentIndex(settings.value("StopBits",QVariant(0)).toInt());
    ui->comboBoxCom5->setCurrentIndex(settings.value("DataBits",QVariant(0)).toInt());
    ui->comboBoxCom6->setCurrentIndex(settings.value("Parrity",QVariant(0)).toInt());
    ui->comboBoxCom7->setCurrentIndex(settings.value("FlowControl",QVariant(0)).toInt());
    ui->checkBoxRx1->setChecked(settings.value("rxHexEnable",false).toBool());
    ui->checkBoxRx2->setChecked(settings.value("vRxTimerStamp",false).toBool());
    //发送窗口的数据
    ui->plainTextTx->TextTxBuff=settings.value("TxPlainText","").toByteArray();
    //多条发送窗口
    for(qint16 i=0;i<SerialMutipSendNum;i++)
    {
        LineEditData[i] = settings.value(QString("vLineText%1").arg(i+1),"").toByteArray();
    }
    ui->checkBox_1->setChecked(settings.value("vTxMultiple",false).toBool());
    ui->checkBox_2->setChecked(settings.value("vTxHexEn",false).toBool());
    ui->checkBox_3->setChecked(settings.value("vTxStamp",false).toBool());
    ui->spinBox->setValue(settings.value("TxTimerCnt",1).toInt());
    ui->spinBox_2->setValue(settings.value("TxSeaskyTimerCnt",1).toInt());


    /******************************恢复大部分数据******************************/
    QStringList list;
    list<<"*.ini";
    QString Path =  qApp->applicationDirPath()+ModulePath;
    QStringList strlist=vGetFilrName(Path,list);
    for(int i = 0;i<strlist.size();i++)
    {
        QString setItem = strlist[i];
        setItem.replace(".ini","");
        ui->comboBox->addItem(setItem);
    }
    ui->comboBox->addItem(QString::fromLocal8Bit("增加模块"));
    if(settings.value("ModuleSelection",QVariant(0)).toInt()<ui->comboBox->count()-1)
    {
        ui->comboBox->setCurrentText((settings.value("comboBox",QVariant(0)).toString()));
        if(settings.value("ModuleSelection",QVariant(0)).toString()!="增加模块")
        {
            vModuleChanged(ui->comboBox->currentIndex());
        }
    }
    settings.endGroup();
}
//关闭时保存设置
void MainWindow::vWriteSettings(void)
{
    vSaveModule();
    QString path = qApp->applicationDirPath()+
            CfgPath+"/"+"config.ini";
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("Configuration");
    settings.setValue("size",size());
    settings.setValue("pos",pos());
    //串口配置
    settings.setValue("PortName",ui->comboBoxCom2->currentIndex());
    settings.setValue("BaudRate",ui->comboBoxCom3->currentIndex());
    settings.setValue("StopBits",ui->comboBoxCom4->currentIndex());
    settings.setValue("DataBits",ui->comboBoxCom5->currentIndex());
    settings.setValue("Parrity",ui->comboBoxCom6->currentIndex());
    settings.setValue("FlowControl",ui->comboBoxCom7->currentIndex());
    settings.setValue("rxHexEnable",ui->checkBoxRx1->isChecked());
    settings.setValue("vRxTimerStamp",ui->checkBoxRx2->isChecked());
    //发送窗口的数据
    settings.setValue("TxPlainText",ui->plainTextTx->TextTxBuff);
    //多条发送窗口
    for(qint16 i=0;i<SerialMutipSendNum;i++)
    {
        settings.setValue(QString("vLineText%1").arg(i+1),LineEditData[i]);
    }
    settings.setValue("vTxMultiple",ui->checkBox_1->isChecked());
    settings.setValue("vTxHexEn",ui->checkBox_2->isChecked());
    settings.setValue("vTxStamp",ui->checkBox_3->isChecked());

    settings.setValue("TxTimerCnt",ui->spinBox->value());
    settings.setValue("TxSeaskyTimerCnt",ui->spinBox_2->value());
    settings.setValue("ModuleSelection",ui->comboBox->currentText());

    settings.endGroup();
}
//自定义控件触发
void MainWindow::vControlCmd(qint16 cmdID)
{
    switch (cmdID)
    {
        case 1:{
            vOpenGlWidget.show();
        };break;
        case 2:{};break;
        case 3:{};break;
        case 4:{};break;
        case 5:{};break;
        default:break;
    }
}
//设置显示接收、发送统计
void MainWindow::vStatusbarCfg(void)
{
    static uint8_t firstInto = 0;
    static QLabel *vInfo[3];
    if(firstInto==0)
    {
        firstInto=1;
        QWidget     * vQWidget     = new QWidget(ui->statusbar);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);
        QPushButton * vButton = new QPushButton(vQWidget);
        vQHBoxLayout->setMargin(6);
        for(qint8 i=0;i<3;i++)
        {
            vInfo[i] = new QLabel;
            vInfo[i]->setFrameStyle(QFrame::Box|QFrame::Sunken);
            vQHBoxLayout->addWidget(vInfo[i]);
        }
        vInfo[2]->setMinimumSize(72,0);
        vInfo[2]->setMaximumSize(72,32);
        vButton->setMinimumSize(32,0);
        vButton->setMaximumSize(72,32);
        vButton->setText(QString::fromLocal8Bit("复位统计"));
        vQHBoxLayout->addWidget(vButton);
        vInfo[0]->setText(QString::fromLocal8Bit("接收统计:%1")
                          .arg(this->vSerialCtr.vSerial.vSerialData->rxByteCnt));
        vInfo[1]->setText(QString::fromLocal8Bit("发送统计:%1")
                          .arg(this->vSerialCtr.vSerial.vSerialData->txByteCnt));
        vInfo[2]->setText(QString::fromLocal8Bit("<a href=\"https://seasky-master.github.io/SEASKY-Master/\">个人博客</a>"));
        vInfo[2]->setOpenExternalLinks(true);//设置可以打开网站链接
        vQWidget->setLayout(vQHBoxLayout);   //显示永久信息
        ui->statusbar->setMinimumSize(512,44);
        vQWidget->setMinimumSize(512,32);
        ui->statusbar->addPermanentWidget(vQWidget);
        connect(&this->vRxTxInfoTimer,&QTimer::timeout,
                this,&MainWindow::vStatusbarCfg);
        vRxTxInfoTimer.start(100);
        connect(vButton,&QPushButton::released,[=]()
        {
            this->vSerialCtr.vSerial.vClearCntNum();
        });
    }
    else
    {
        vInfo[0]->setText(QString::fromLocal8Bit("接收统计:%1")
                          .arg(this->vSerialCtr.vSerial.vSerialData->rxByteCnt));
        vInfo[1]->setText(QString::fromLocal8Bit("发送统计:%1")
                          .arg(this->vSerialCtr.vSerial.vSerialData->txByteCnt));
    }
}
/*保存接收窗口数据，注意只是保存窗口内加载的数据，如果要保存所有数据，需要手动加载全部*/
void MainWindow::vSaveRxText(void)
{
    QString dirpath =
       QFileDialog::getSaveFileName(this, QStringLiteral("Save"),
                                    qApp->applicationDirPath(),
                                    QString(tr("File (*.txt)")),
                                    Q_NULLPTR,
                                    QFileDialog::ShowDirsOnly |
                                    QFileDialog::DontResolveSymlinks);
    if(dirpath!=NULL)
    {
        QFile file(dirpath);
        //方式：Append为追加，WriteOnly，ReadOnly
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::critical(NULL,
                                  QString::fromLocal8Bit("提示"),
                                  QString::fromLocal8Bit("无法创建文件"));
            return ;
        }
        QTextStream out(&file);
        out<<ui->plainTextRx->toPlainText();
    }
}
//获取路径中需要的文件列表
QStringList MainWindow::vGetFilrName(const QString &strPath,const QStringList &filters)
{
    //判断路径是否存在
    QDir dir(strPath);
    if(!dir.exists())
    {
        dir.mkpath(strPath);
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
    dir.setNameFilters(filters);  //设置文件名称过滤器
    //统计文件个数
    int count = dir.count();
    QStringList name_list;
    for(int i = 0; i < count; i++)
    {
        QString file_name= dir[i];  //文件名称
        name_list.append(file_name);
    }
    return name_list;
}
//保存协议接收
void MainWindow::vSaveRxSeaskyText(void)
{
    QString dirpath =
            QFileDialog::getSaveFileName(this,
                                         QStringLiteral("Save"),
                                         qApp->applicationDirPath(),
                                         QString(tr("File (*.txt)")),
                                         Q_NULLPTR,
                                         QFileDialog::ShowDirsOnly |
                                         QFileDialog::DontResolveSymlinks);
    if(dirpath!=NULL)
    {
        QFile file(dirpath);
        //方式：Append为追加，WriteOnly，ReadOnly
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::critical(NULL,
                                  QString::fromLocal8Bit("提示"),
                                  QString::fromLocal8Bit("无法创建文件"));
            return ;
        }
        QTextStream out(&file);
        out<<ui->plainTextEdit->toPlainText();
    }
}
//刷新接收槽函数
void MainWindow::vRxSlotChanged(void)
{
    if(ui->pushButton_4->isChecked())
    {
        this->rxModeCfg = true;
    }
    else
    {
        this->rxModeCfg = false;
    }
    if(this->rxModeCfg)//连接为协议接收
    {
        //避免重复创建
        this->vSerialCtr.vQObjectRxCtr.vDisConnectRx();
        this->vSerialCtr.vSeaskyPortCtr.vDisConnectRx();
        this->vSerialCtr.vSeaskyPortCtr.vConnectRx();
    }
    else //连接为串口调试助手接收
    {
        //避免重复创建
        this->vSerialCtr.vQObjectRxCtr.vDisConnectRx();
        this->vSerialCtr.vSeaskyPortCtr.vDisConnectRx();
        this->vSerialCtr.vQObjectRxCtr.vConnectRx();
    }
}
//根据配置更新槽函数,以及更新显示
void MainWindow::vTxSlotChanged(void)
{
    if(txModeCfg)
    {
        //设置vQObjectTxCtr的发送定时器无法启动
        this->vSerialCtr.vQObjectTxCtr.vTimerStop();
        ui->checkBox_4->setChecked(false);
        ui->checkBox_4->setEnabled(false);
        //先断开所有，避免重复套用了链接
        this->vSerialCtr.vQObjectTxCtr.vDisConnectTx();
        this->vSerialCtr.vSeaskyPortCtr.vDisConnectTx();
        this->vSerialCtr.vSeaskyPortCtr.vConnectTx();
    }
    else
    {
        this->vSerialCtr.vQObjectTxCtr.vDisConnectTx();
        this->vSerialCtr.vSeaskyPortCtr.vDisConnectTx();
        this->vSerialCtr.vQObjectTxCtr.vConnectTx();
        //恢复vQObjectTxCtr的发送定时器权力
        ui->checkBox_4->setEnabled(true);
    }
}
//刷新Seasky协议模块
void MainWindow::vModuleChanged(qint16 index)
{
    if(ui->comboBox->currentText()==QString::fromLocal8Bit("增加模块"))
    {
        vModuleAddItem();
    }
    else
    {
        vShowModule(index);
    }
}
//刷新接收时间戳使能
void MainWindow::vRxTimerStampChanged(void)
{
    if(ui->checkBoxRx2->isChecked())
    {
        this->vSerialCtr.vSerial.vSerialData->rxTimeStampEnable = true;
    }
    else
    {
        this->vSerialCtr.vSerial.vSerialData->rxTimeStampEnable = false;
    }
}
//刷新协议帧头数据-RX
void MainWindow::showRxHead(void)
{
    ui->widgetScope2->vSetFlag(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vReg);
    ui->lineEdit_12->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vCmdId,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_14->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vReg,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_16->setText(QString::number(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataLen,10));
}
//刷新协议帧头数据-TX
void MainWindow::showTxHead(void)
{
    ui->lineEdit_4->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vCmdId,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_6->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vReg,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_8->setText(QString::number(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataLen,10));
}
//刷新协议寄存器数据
void MainWindow::vPortShow(void)
{
    ui->widgetScope1->vSetFlag(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vReg);
    ui->widgetScope2->vSetFlag(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vReg);
    showRxHead();
    showTxHead();
    emit this->vSerialCtr.vSeaskyPortCtr.vQWidgetRxShow();
    emit this->vSerialCtr.vSeaskyPortCtr.vQWidgetTxShow();
}
//刷新RxHexEnable
void MainWindow::vRxHexEnableCfg(void)
{
    if(ui->checkBoxRx1->isChecked())
    {
        *ui->plainTextRx->hexEnable = true;
    }
    else
    {
        *ui->plainTextRx->hexEnable = false;
    }
    emit plaintextEditShowOne();
}
//刷新TxHexEnable
void MainWindow::vTxHexEnableCfg(void)
{
    if(ui->checkBox_2->isChecked())
    {
        this->vSerialCtr.vSerial.vSerialData->txHexEnable = true;
    }
    else
    {
        this->vSerialCtr.vSerial.vSerialData->txHexEnable = false;
    }
    emit txHexEnableChanged();
}
//模式切换处理
void MainWindow::vTxModeCfg(void)
{
    //重新设置定时器的值
    this->vSerialCtr.vSeaskyPortCtr.setVtimerTxCnt(ui->spinBox_2->value());
    if(ui->pushButton_3->isChecked())
    {
        //按协议发送
        this->vSerialCtr.vQObjectTxCtr.vSerialTxMode = SerialAgr;
        //协议使能
        txModeCfg = true;
        this->vTxSlotChanged();
        //模式改变，重启定时器
        this->vSerialCtr.vSeaskyPortCtr.vQTimerTxStop();
        this->vSerialCtr.vSeaskyPortCtr.vQTimerTxStart();
    }
    else
    {
        if(!ui->checkBox_1->isChecked())
        {
            //单条发送
            this->vSerialCtr.vQObjectTxCtr.vSerialTxMode = SerialOrd;
        }
        else
        {
            //多条发送
            this->vSerialCtr.vQObjectTxCtr.vSerialTxMode = SerialMul;
        }
        //协议失能
        txModeCfg = false;
        this->vTxSlotChanged();
        //关闭定时器，协议发送定时器
        this->vSerialCtr.vSeaskyPortCtr.vQTimerTxStop();
    }
}
//发送换行符控制
void MainWindow::vTxStampCfg(void)
{
    if(ui->checkBox_3->isChecked())
    {
        this->vSerialCtr.vQObjectTxCtr.LineFeed=QByteArray("\n");
    }
    else
    {
        this->vSerialCtr.vQObjectTxCtr.LineFeed=QByteArray("");
    }
}
//发送的定时器控制
void MainWindow::vTxModeTimerCfg(void)
{
    this->vSerialCtr.vQObjectTxCtr.vTimerSet(ui->spinBox->value());
    if(!txModeCfg)
    {
        if(ui->checkBox_4->isChecked())
        {
            this->vSerialCtr.vQObjectTxCtr.vTimerStop();
            this->vSerialCtr.vQObjectTxCtr.vTimerStart();
        }
        else
        {
            this->vSerialCtr.vQObjectTxCtr.vTimerStop();
        }
    }
}
//同步窗口和变量数据
void MainWindow::vUpdateShow(void)
{
    //以下函数根据参数变化更新显示
    vRxSlotChanged();        //刷新接收槽函数连接
    vTxSlotChanged();        //刷新发送槽函数连接
    vRxTimerStampChanged();  //刷新接收时间戳使能
    showRxHead();            //刷新协议RX显示
    showTxHead();            //刷新协议TX显示
    vPortShow();             //刷新协议显示
    vRxHexEnableCfg();       //刷新RxHexEnable
    vTxHexEnableCfg();       //刷新TxHexEnable
    vTxModeCfg();            //发送模式切换处理
    vTxStampCfg();           //发送换行符控制
    vTxModeTimerCfg();       //发送的定时器控制
}
//刷新协议模块
void MainWindow::vShowModule(qint16 index)
{
    if(ui->comboBox->itemText(index)!=
            QString::fromLocal8Bit("增加模块")&&
            ui->comboBox->itemText(index)!="")
    {
        QString path = qApp->applicationDirPath()+ModulePath+"/"+
                ui->comboBox->currentText()+".ini";
        QSettings settingsModule(path,QSettings::IniFormat);
        settingsModule.beginGroup("Module");
        for(qint16 i=0;i<SeaskyPortNum;i++)
        {
            this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vName[i] =
            settingsModule.value(QString("vNameRx:%1").arg(i+1),
                                 QString("Name:%1").arg(i+1)).toString();
            this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vUnit[i] =
            settingsModule.value(QString("vUnitRx:%1").arg(i+1),
                                 QString("Unit:%1").arg(i+1)).toString();
            this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vQString[i] =
            settingsModule.value(QString("vQStringRx:%1").arg(i+1),
                                 QString("0").arg(i+1)).toString();
            this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vFloat[i] =
            settingsModule.value(QString("vFloatRx:%1").arg(i+1),
                                 QString("0").arg(i+1)).toFloat();
        }
        this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vCmdId =
            settingsModule.value(QString("vCmdIdRx"),
                                 QString("%1").arg(0X0001)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vReg =
            settingsModule.value(QString("vRegRx"),
                                 QString("%1").arg(0X00FF)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataLen =
            settingsModule.value(QString("vDataLenRx"),
                                 QString("0")).toInt();

        for(qint16 i=0;i<SeaskyPortNum;i++)
        {
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vName[i] =
            settingsModule.value(QString("vNameTx:%1").arg(i+1),
                                 QString("Name:%1").arg(i+1)).toString();
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vUnit[i] =
            settingsModule.value(QString("vUnitTx:%1").arg(i+1),
                                 QString("Unit:%1").arg(i+1)).toString();
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vQString[i] =
            settingsModule.value(QString("vQStringTx:%1").arg(i+1),
                                 QString("0").arg(i+1)).toString();
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vFloat[i] =
            settingsModule.value(QString("vFloatTx:%1").arg(i+1),
                                 QString("0").arg(i+1)).toFloat();
        }
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vCmdId =
            settingsModule.value(QString("vCmdIdTx"),
                                 QString("%1").arg(0X0001)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vReg =
            settingsModule.value(QString("vRegTx"),
                                 QString("%1").arg(0X00FF)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataLen =
            settingsModule.value(QString("vDataLenTx"),
                                 QString("0")).toInt();
        settingsModule.endGroup();
        vPortShow();
//      EnableModuleSave = true;
    }
}
//添加SEASKY协议模块
void MainWindow::vModuleAddItem(void)
{
    bool ok,nohave;
    int i;
    QString str;
    ok = false;
    nohave = false;
    str =
    QInputDialog::getText(this,QString::fromLocal8Bit("增加模块"),
                  QString::fromLocal8Bit("新的模块名称"),
                  QLineEdit::Normal,"vName",
                  &ok,Qt::WindowCloseButtonHint);
    for(i=0;i<ui->comboBox->count();i++)
    {
        if(ok)
        {
            if(ui->comboBox->itemText(i)==str)//如果已经有了
            {
                nohave = false;
                break;
            }
        }
    }
    if(i>=ui->comboBox->count()){nohave = true;}
    if(nohave&ok)
    {
        ui->comboBox->setItemText(ui->comboBox->count()-1,str);
        ui->comboBox->addItem(QString::fromLocal8Bit("增加模块"));
        vShowModule(ui->comboBox->currentIndex());
        vSaveModule();
    }
    else if(!nohave)
    {
        doWarning(QString::fromLocal8Bit("该名称已被使用"));
    }
}
//保存SEASKY协议模块数据
void MainWindow::vSaveModule(void)
{
    if((ui->comboBox->currentText()!=QString::fromLocal8Bit("增加模块"))&&(ui->comboBox->currentText()!=""))
    {
        QString path = qApp->applicationDirPath()+
                ModulePath+"/"+
                ui->comboBox->currentText()+".ini";
        QSettings settingsModule(path,QSettings::IniFormat);
        settingsModule.beginGroup("Module");
        for(qint16 i=0;i<SeaskyPortNum;i++)
        {
            settingsModule.setValue(QString("vNameTx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vName[i]);
            settingsModule.setValue(QString("vUnitTx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vUnit[i]);
            settingsModule.setValue(QString("vQStringTx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vQString[i]);
            settingsModule.setValue(QString("vFloatTx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vFloat[i]);
        }
        settingsModule.setValue(QString("vCmdIdTx"),
                                this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vCmdId);
        settingsModule.setValue(QString("vRegTx"),
                                this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vReg);
        settingsModule.setValue(QString("vDataLenTx"),
                                this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataLen);
        for(qint16 i=0;i<SeaskyPortNum;i++)
        {
            settingsModule.setValue(QString("vNameRx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vName[i]);
            settingsModule.setValue(QString("vUnitRx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vUnit[i]);
            settingsModule.setValue(QString("vQStringRx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vQString[i]);
            settingsModule.setValue(QString("vFloatRx:%1").arg(i+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vFloat[i]);
        }
        settingsModule.setValue(QString("vCmdIdRx"),
                                this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vCmdId);
        settingsModule.setValue(QString("vRegRx"),
                                this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vReg);
        settingsModule.setValue(QString("vDataLenRx"),
                                this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataLen);

        settingsModule.endGroup();
    }
}
void MainWindow::vTabTimerCfg(void)
{
    /*  显示相关的定时器
     *0.ui->plainTextRx   有一个定时器
     *1.ui->plainTextEdit 有一个定时器
     * .this->vSerialCtr.vSeaskyPortCtr.vQTimer 刷新界面定时器
     *2.widgetScope       有一个定时器
     *
     */
    switch(ui->tabWidget1->currentIndex())
    {
        case 0:
        {
            if(ui->plainTextRx->TimerEnable == true)
            {
                ui->plainTextRx->TimerCtr.start();
            }
            if(ui->plainTextEdit->TimerEnable == true)
            {
                ui->plainTextEdit->TimerCtr.stop();
            }
            if(ui->widgetScope->TimerEnable == true)
            {
                ui->widgetScope->GraphShowTimer.stop();
            }
            if(this->vSerialCtr.vSeaskyPortCtr.vQTimerEnable == true)
            {
                this->vSerialCtr.vSeaskyPortCtr.vQTimer.stop();
            }

        };break;//串口调试助手界面
        case 1:
        {
            if(ui->plainTextRx->TimerEnable == true)
            {
                ui->plainTextRx->TimerCtr.stop();
            }
            if(ui->plainTextEdit->TimerEnable == true)
            {
                ui->plainTextEdit->TimerCtr.start();
            }
            if(ui->widgetScope->TimerEnable == true)
            {
                ui->widgetScope->GraphShowTimer.stop();
            }
            if(this->vSerialCtr.vSeaskyPortCtr.vQTimerEnable == true)
            {
                this->vSerialCtr.vSeaskyPortCtr.vQTimer.start();
            }
        };break;//Seasky界面
        case 2:
        {
            if(ui->plainTextRx->TimerEnable == true)
            {
                ui->plainTextRx->TimerCtr.stop();
            }
            if(ui->plainTextEdit->TimerEnable == true)
            {
                ui->plainTextEdit->TimerCtr.stop();
            }
            if(ui->widgetScope->TimerEnable == true)
            {
                ui->widgetScope->GraphShowTimer.start();
            }
            if(this->vSerialCtr.vSeaskyPortCtr.vQTimerEnable == true)
            {
                this->vSerialCtr.vSeaskyPortCtr.vQTimer.stop();
            }
        };break;//串口示波器界面
        default:break;
    }
}
void MainWindow::vImportLineText(QString str)
{
    if(str!=NULL)
    {
        QFile vQfile(str);
        //读取文件配置发送窗口
        if (!vQfile.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(NULL,
                                  QString::fromLocal8Bit("提示"),
                                  QString::fromLocal8Bit("无法打开该文件"));
            return ;
        }
        else
        {
            //文本解析操作
            QStringList list;
            qint32      vFindCom = -1;
            list.clear();
            QTextStream csvStream(&vQfile);
            //先读取一行
            QString fileLine = csvStream.readLine();
            list =  fileLine.split(",",
                                  QString::SkipEmptyParts);
            for(int j=0;j<list.size();j++)
            {
                if(FindSerialCommand==QString(list[j]))
                {
                    vFindCom = j;
                }
            }
            if(vFindCom>=0)
            {
                for(int i = 0;!csvStream.atEnd();i++)
                {
                    QString fileLine = csvStream.readLine();
                    if(i<SerialMutipSendNum)
                    {

                        list =  fileLine.split(",",
                                              QString::KeepEmptyParts);
                        LineEditData[i] = (list[vFindCom]).toLocal8Bit();
                        vTxHexEnableCfg();
                    }
                    else
                    {
                        doWarning(
                        QString::fromLocal8Bit("达到最多设定数量!（该多条发送最多仅支持添加%1条指令,多余指令未添加。）").arg(SerialMutipSendNum));
                        break;
                    }
                }
            }
            else
            {
                doWarning(QString::fromLocal8Bit("请检查文件格式是否正确!"));
            }
        }
    }
}
void MainWindow::vTxError(void)
{
    vSerialStatusCheck();
}
//串口状态检测
void MainWindow::vSerialStatusCheck(void)
{
    if(this->vSerialCtr.vSerial.qSerial->isOpen())
    {
        this->SerialOpen();
    }
    else
    {
        this->SerialClose();
    }
}
/*---------------------------提示窗口-------------------------------*/
/*错误*/
void MainWindow::doCritical(const QString &str)
{
    QMessageBox::critical(NULL,QString::fromLocal8Bit("错误"),str);
}
/*警告*/
void MainWindow::doWarning(const QString &str)
{
    QMessageBox::warning(this,QString::fromLocal8Bit("警告"),str);
}
/*帮助*/
void MainWindow::doHelp(void)
{
    QMessageBox::information(this,QString::fromLocal8Bit("帮助"),"");
}
/*关于*/
void MainWindow::doAbout(void)
{
    QMessageBox::about(this,QString::fromLocal8Bit("关于"),
                            QString::fromLocal8Bit("作者：SEASKY-刘威\n"
                                                   "参考：\n"));
}
