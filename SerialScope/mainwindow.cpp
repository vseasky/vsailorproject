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
#include <vqtexteditline.h>
#include "vansibash.h"
#include "CSVHelper.h"
#include <QDebug>

#ifdef WinDownVersion
#pragma comment(lib,"user32.lib")
#endif

/*-----------界面刷新相关的定时器-----------*/
const qint32 vTimerRxPlainText   = 100;
const qint32 vTimerRxPlainSeasky = 100;
const qint32 vTimerRxSeasky      = 100;
const qint32 vTimerRxScope       = 75;
const qint32 vTimerRxPlainServer = 100;
const qint16  TcpServerMutipSendNum = 200;      //串口调试助手多条发送模式条目数量
const qint16  SerialMutipSendNum = 200;         //串口调试助手多条发送模式条目数量
const qint16  SeaskyPortNumMutipSendNum = 100;  //串口协议最大存储命令数
const qint16  SerialLinuxNum     = 25;
const qint32  SerialTxTimerCfg   = 100;//默认串口发送周期，可以通过spinBox调整
const qint32  SeaskyPortNum      = 24; //最大支持的FLOAT数据长度
const qint32  Utf8MaxLen         = 12+SeaskyPortNum*4;
const qint32  SeaskyTimer        = 100;//协议窗口数据刷新频率
const QString FindSerialCommand  = "SerialCommand";
const QString FindTcpCommand     = "TcpCommand";
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
    //TCP相关，2021/9/9日新增
    vTcpServerInit();

    vReadSettings();
    /*更新一次界面显示*/
    vUpdateShow();

    vTcpCfgUpdata();
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
    /*先关闭服务器相关的内容，再关闭程序*/
    //TCP如果不关闭就退出会报异常退出，TCP关闭时会触发断开连接信号，
    //如果绑定了该信号，断开时触发的信号也会导致异常退出
    this->vServerTcp.vServerTcpCtr.vServerTcp.vDisEnConnect();
    this->vServerTcp.vServerTcpCtr.vServerTcp.vCloseServer();

    vWriteSettings();
    event->accept();
}

//0. 配置相关地址，应包含所有地址初始化操作
void MainWindow::vDependenceAddr(void)
{
    /*波形显示控件波形名称查询地址*/
    ui->widgetScope->vSetNameAddr(&vRxName[0]);
    /*协议操作地址受此分配*/
    this->vSerialCtr.vSeaskyPortCtr.vProtocol.ProtocolInitRx((uint32_t*)vRxfloat,vRxUtf8,SeaskyPortNum);
    this->vSerialCtr.vSeaskyPortCtr.vProtocol.ProtocolInitTx((uint32_t*)vTxfloat,vTxUtf8,SeaskyPortNum);

    /*Seaky 协议使用相关数据*/
    //设置依赖的两个Widget 16进制显示窗口
    this->vSerialCtr.vSeaskyPortCtr.configQWidgetEditTx(ui->scrollAreaWidgetContents_7,SeaskyPortNum,SeaskyPortNumMutipSendNum);
    this->vSerialCtr.vSeaskyPortCtr.setQWidgetAddr(
                ui->scrollAreaWidgetContents_2,
                ui->scrollAreaWidgetContents_3);
    this->vSerialCtr.vSeaskyPortCtr.setRxSeaskyAddr(
                &vRxQString[0],&vRxName[0],&vRxUnit[0],&vRxfloat[0]);
    this->vSerialCtr.vSeaskyPortCtr.setTxSeaskyAddr(
                &vTxQString[0],&vTxName[0],&vTxUnit[0],&vTxfloat[0]);

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
                              "color:#FF4500;");
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
                                 "color:#FF4500;");
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
        connect(this,&MainWindow::vMapUpdata,[=]()
        {
            if(LineEditData[i].size()>0)
            {
                vcheckBox->setChecked(*LineEditMap->find(i+1).value());
            }
            else
            {
                *LineEditMap->find(i+1).value() = false;
            }
        });
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
        switch(ui->tabWidget->currentIndex())
        {
            case 0:
            {
                ui->plainTextTx->TextTxBuff.clear();
                //更新显示
                emit txHexEnableChanged();
            }break;
            case 1:
            {
                for(qint16 i=0;i<this->MultPleMaxCnt;i++)
                {
                    LineEditData[i].clear();
                }
                //更新显示
                emit txHexEnableChanged();
            }break;
            case 2:
            {

            }break;
        }
    });
    vServerLinuxCfg();
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
    this->vSerialCtr.vSerial.vSerialConfig->vSerialBaudRate =
            ui->comboBoxCom2->currentData().toInt();
    connect(ui->comboBoxCom2,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialBaudRate =
                ui->comboBoxCom2->currentData().toInt();
    });
    //串口数据位
    this->vSerialCtr.vSerial.vSerialConfig->vSerialStopBits =
            QSerialPort::StopBits(ui->comboBoxCom3->currentData().toInt());
    connect(ui->comboBoxCom3,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialStopBits =
                QSerialPort::StopBits(ui->comboBoxCom3->currentData().toInt());
    });
    this->vSerialCtr.vSerial.vSerialConfig->vSerialDataBits =
            QSerialPort::DataBits(ui->comboBoxCom4->currentData().toInt());
    connect(ui->comboBoxCom4,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialDataBits =
                QSerialPort::DataBits(ui->comboBoxCom4->currentData().toInt());
    });
    //串口极性
    this->vSerialCtr.vSerial.vSerialConfig->vSerialParrity  =
            QSerialPort::Parity(ui->comboBoxCom5->currentData().toInt());
    connect(ui->comboBoxCom5,&QComboBox::currentTextChanged,[=]()
    {
        this->vSerialCtr.vSerial.vSerialConfig->vSerialParrity  =
                QSerialPort::Parity(ui->comboBoxCom5->currentData().toInt());
    });
    //串口编码格式
    *this->vSerialCtr.vSerial.vCodeConver =
            vSerialCodeModeEnum(ui->comboBoxCom6->currentData().toInt());
    connect(ui->comboBoxCom6,&QComboBox::currentTextChanged,[=]()
    {
        *this->vSerialCtr.vSerial.vCodeConver =
                vSerialCodeModeEnum(ui->comboBoxCom6->currentData().toInt());
    });
    //串口流控
    this->vSerialCtr.vSerial.vSerialConfig->vSerialFlowControl =
            QSerialPort::FlowControl(ui->comboBoxCom7->currentData().toInt());
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
    ui->lineEdit_2->setValidator(validator);
    ui->lineEdit_4->setValidator(validator);
    ui->lineEdit_6->setValidator(validator);
    QValidator *IntValidator=new QIntValidator(0,99,this);
    ui->lineEdit_8->setValidator(IntValidator);
    ui->lineEdit_8->setPlaceholderText("请输入1-255");
//    ui->lineEdit_8->setValidator(validator2);
    /*-------------正则表达式,限制输入----------------*/
    //TX的CMD_ID数据更新
    connect(ui->lineEdit_2,&QLineEdit::editingFinished,[=]()
    {
        QByteArray addr;
        QString str= ui->lineEdit_2->text();
        if(str.length()==6)
        {
            addr=QByteArray::fromHex(
                    ui->lineEdit_2->text().mid(2,6).toUtf8());
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentType =
                    ((addr[1])& 0x00FF)|((addr[0] << 8) & 0xFF00);
            //配置保存到配置文件
            vSaveModule();
        }
    });
    //TX的CMD_ID数据更新
    connect(ui->lineEdit_4,&QLineEdit::editingFinished,[=]()
    {
        QByteArray addr;
        QString str= ui->lineEdit_4->text();
        if(str.length()==6)
        {
            addr=QByteArray::fromHex(
                    ui->lineEdit_4->text().mid(2,6).toUtf8());
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentId =
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
            this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataId =
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
//    ui->widget_2->();
    //模型改变
    void (QComboBox::*vChanged)(int)=&QComboBox::activated;
    connect(ui->comboBox,vChanged,
            this,&MainWindow::vModuleChanged);
    //数据改变，保存配置
    connect(&this->vSerialCtr.vSeaskyPortCtr,
            &vSeaskyPort::vInfoChanged,
            this,&MainWindow::vSaveModule);

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

    vQVBoxLayout->setAlignment(Qt::AlignTop);

    QPushButton   * vQPushButton1;
    QPushButton   * vQPushButton2;
    //创建8个
    for(qint16 i=0;i<4;i++)
    {
        QWidget     * vQWidget = new QWidget(ui->scrollAreaWidgetContents_4);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);

        vQPushButton1 = new QPushButton(vQWidget);
        vQPushButton2 = new QPushButton(vQWidget);
        vQPushButton1->setMinimumSize(64,64);
        vQPushButton1->setMaximumSize(64,64);
        vQPushButton2->setMinimumSize(64,64);
        vQPushButton2->setMaximumSize(64,64);
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
    this->vSerialCtr.vSerial.vSerialConfig->vSerialPortName    = ui->comboBoxCom1->currentData().toString();
    this->vSerialCtr.vSerial.vSerialConfig->vSerialBaudRate    = ui->comboBoxCom2->currentData().toInt();
    this->vSerialCtr.vSerial.vSerialConfig->vSerialStopBits    = QSerialPort::StopBits(ui->comboBoxCom3->currentData().toInt());
    this->vSerialCtr.vSerial.vSerialConfig->vSerialDataBits    = QSerialPort::DataBits(ui->comboBoxCom4->currentData().toInt());
    this->vSerialCtr.vSerial.vSerialConfig->vSerialParrity     = QSerialPort::Parity(ui->comboBoxCom5->currentData().toInt());
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
        *LineEditMap->find(i+1).value() = settings.value(QString("LineEditMap%1").arg(i+1),false).toBool();
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

    /*QTcpServer QComboBox*/
    ui->comboBoxServer6->setCurrentIndex(settings.value("comboBoxServer6",0).toInt());
    ui->comboBoxTcpL->setCurrentIndex(settings.value("comboBoxTcpL",0).toInt());

    /*QTcpServer QCheckBox*/
    ui->vSreverRxHex->setChecked(settings.value("vSreverRxHex",false).toBool());
    ui->vSreverRxTim->setChecked(settings.value("vSreverRxTim",false).toBool());
    ui->vTcpServerTxs->setChecked(settings.value("vTcpServerTxs",false).toBool());
    ui->vTcpServerTxHex->setChecked(settings.value("vTcpServerTxHex",false).toBool());
    ui->vTcpServeradd->setChecked(settings.value("vTcpServeradd",false).toBool());
    /*QTcpServer QSpinBox*/
    ui->vTcpServerTimCnt->setValue(settings.value("vTcpServerTimCnt",100).toInt());
    ui->textEdit->TextTxBuff = settings.value("textEdit","").toByteArray();
    //多条Tcp发送框
    for(qint16 i=0;i<TcpServerMutipSendNum;i++)
    {
        this->vTcpLineEditData[i] = settings.value(QString("TcpServerLine%1").arg(i+1),"").toByteArray();
        *vTcpLineEditMap->find(i+1).value() = settings.value(QString("vTcpLineEditMap%1").arg(i+1),false).toBool();
    }
    //记录个数
    qint16 ServerCount = 0;
    ServerCount = settings.value("ServerCount",0).toInt();
    if(ui->comboBoxServer1->currentText()==QString::fromLocal8Bit("添加端口号"))
    {
        ui->comboBoxServer1->clear();
        for(qint16 i=0;i<ServerCount;i++)
        {
            ui->comboBoxServer1->addItem(
                        settings.value(QString("ServeritemText%1").arg(i),0).toString(),
                        settings.value(QString("ServeritemData%1").arg(i),0).toInt());
        }
        ui->comboBoxServer1->addItem(QString::fromLocal8Bit("添加端口号"),0);
    }
    if(settings.value("ServerIndex",0).toInt()<ui->comboBoxServer1->count())
    {
        ui->comboBoxServer1->setCurrentIndex(settings.value("ServerIndex",0).toInt());
    }
    for(qint16 i=0;i<SerialLinuxNum;i++)
    {
        this->vLinuxData[i] = settings.value(QString("vLinuxData%1").arg(i+1),"").toByteArray();
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
        settings.setValue(QString("LineEditMap%1").arg(i+1),*LineEditMap->find(i+1).value());
    }
    settings.setValue("vTxMultiple",ui->checkBox_1->isChecked());
    settings.setValue("vTxHexEn",ui->checkBox_2->isChecked());
    settings.setValue("vTxStamp",ui->checkBox_3->isChecked());

    settings.setValue("TxTimerCnt",ui->spinBox->value());
    settings.setValue("TxSeaskyTimerCnt",ui->spinBox_2->value());
    settings.setValue("ModuleSelection",ui->comboBox->currentText());

    /*QTcpServer QComboBox*/
    settings.setValue("comboBoxServer6",ui->comboBoxServer6->currentIndex());
    settings.setValue("comboBoxTcpL",ui->comboBoxTcpL->currentIndex());
    /*QTcpServer QCheckBox*/
    settings.setValue("vSreverRxHex",ui->vSreverRxHex->isChecked());
    settings.setValue("vSreverRxTim",ui->vSreverRxTim->isChecked());
    settings.setValue("vTcpServerTxs",ui->vTcpServerTxs->isChecked());
    settings.setValue("vTcpServerTxHex",ui->vTcpServerTxHex->isChecked());
    settings.setValue("vTcpServeradd",ui->vTcpServeradd->isChecked());
    settings.setValue("vTcpServerTimCnt",ui->vTcpServerTimCnt->text());

    //Tcp发送窗口的数据
    settings.setValue("textEdit",ui->textEdit->TextTxBuff);
    //多条Tcp发送框
    for(qint16 i=0;i<TcpServerMutipSendNum;i++)
    {
        settings.setValue(QString("TcpServerLine%1").arg(i+1),vTcpLineEditData[i]);
        settings.setValue(QString("vTcpLineEditMap%1").arg(i+1),*vTcpLineEditMap->find(i+1).value());
    }
    //记录个数
    settings.setValue("ServerCount",ui->comboBoxServer1->count()-1);
    for(qint16 i=0;i<ui->comboBoxServer1->count()-1;i++)
    {
        settings.setValue(QString("ServeritemText%1").arg(i),
                          ui->comboBoxServer1->itemText(i));
        settings.setValue(QString("ServeritemData%1").arg(i),
                          ui->comboBoxServer1->itemData(i).toInt());
    }
    settings.setValue("ServerIndex",ui->comboBoxServer1->currentIndex());


    for(qint16 i=0;i<SerialLinuxNum;i++)
    {
        settings.setValue(QString("vLinuxData%1").arg(i+1),vLinuxData[i]);
    }

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
        vInfo[2]->setMinimumSize(96,0);
        vInfo[2]->setMaximumSize(96,32);
        vButton->setMinimumSize(96,0);
        vButton->setMaximumSize(96,32);
        vButton->setText(QString::fromLocal8Bit("复位统计"));
        vQHBoxLayout->addWidget(vButton);
        vInfo[0]->setText(QString::fromLocal8Bit("接收统计:%1")
                          .arg(this->vSerialCtr.vSerial.vSerialData->rxByteCnt));
        vInfo[1]->setText(QString::fromLocal8Bit("发送统计:%1")
                          .arg(this->vSerialCtr.vSerial.vSerialData->txByteCnt));
        vInfo[2]->setText(QString::fromLocal8Bit("<a href=\"www.liuwei.vin\">个人博客</a>"));
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
        this->vSeaskyMode = true;
    }
    else
    {
        this->vSeaskyMode = false;
    }
    if(this->vSeaskyMode)//连接为协议接收
    {
        //先断开所有，避免重复套用了链接
        this->vSerialCtr.vQObjectTxCtr.vDisConnectTx();
        this->vSerialCtr.vQObjectRxCtr.vDisConnectRx();
        this->vSerialCtr.vSeaskyPortCtr.vDisConnect();
        this->vSerialCtr.vSeaskyPortCtr.vConnect();
        ui->pushButton_3->setEnabled(true);
    }
    else //连接为串口调试助手接收
    {
        ui->pushButton_3->setChecked(false);
        ui->pushButton_3->setEnabled(false);
        this->vSerialCtr.vQObjectTxCtr.vDisConnectTx();
        this->vSerialCtr.vQObjectRxCtr.vDisConnectRx();
        this->vSerialCtr.vSeaskyPortCtr.vDisConnect();
        this->vSerialCtr.vQObjectRxCtr.vConnectRx();
        this->vSerialCtr.vQObjectTxCtr.vConnectTx();
    }
    vTxSlotChanged();
}
//根据配置更新槽函数,以及更新显示
void MainWindow::vTxSlotChanged(void)
{
    if(vSeaskyMode)
    {
        //设置vQObjectTxCtr的发送定时器无法启动
        this->vSerialCtr.vQObjectTxCtr.vTimerStop();
        ui->checkBox_4->setChecked(false);
        ui->checkBox_4->setEnabled(false);
    }
    else
    {
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
    ui->lineEdit_10->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vEquipmentType,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_12->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vEquipmentId,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_14->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataId,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_16->setText(QString::number(this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataLen,10));
}
//刷新协议帧头数据-TX
void MainWindow::showTxHead(void)
{
    ui->lineEdit_2->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentType,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_4->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentId,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_6->setText("0X"+QString::number(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataId,
                                                  16).toUpper().rightJustified(4, QChar('0')));
    ui->lineEdit_8->setText(QString::number(this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataLen,10));
}
//刷新协议寄存器数据
void MainWindow::vPortShow(void)
{
    showRxHead();
    showTxHead();
    emit this->vSerialCtr.vSeaskyPortCtr.vQWidgetRxShow();
    emit this->vSerialCtr.vSeaskyPortCtr.vQWidgetTxShow();
    emit this->vSerialCtr.vSeaskyPortCtr.vQWidgetTxLineShow1();
    emit this->vSerialCtr.vSeaskyPortCtr.vQWidgetTxLineShow2();
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
        this->vTxSlotChanged();
        ui->pushButton_4->setEnabled(false);
        //模式改变，重启定时器
        this->vSerialCtr.vSeaskyPortCtr.vQTimerTxStop();
        this->vSerialCtr.vSeaskyPortCtr.vQTimerTxStart();
    }
    else
    {
        ui->pushButton_4->setEnabled(true);
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
    if(!vSeaskyMode)
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
    readSerialChange();
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
    vMapUpdata();
    emit vLinuxShow();
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

        this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vEquipmentType =
            settingsModule.value(QString("vEquipmentTypeRx"),
                                 QString("%1").arg(0X0001)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vEquipmentId =
            settingsModule.value(QString("vEquipmentIdRx"),
                                 QString("%1").arg(0X0001)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataId =
            settingsModule.value(QString("vDataIdRx"),
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
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentType =
            settingsModule.value(QString("vEquipmentTypeTx"),
                                 QString("%1").arg(0X0001)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentId =
            settingsModule.value(QString("vEquipmentIdTx"),
                                 QString("%1").arg(0X0001)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataId =
            settingsModule.value(QString("vDataIdTx"),
                                 QString("%1").arg(0X00FF)).toUInt();
        this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataLen =
            settingsModule.value(QString("vDataLenTx"),
                                 QString("0")).toInt();


        for(qint16 k=0;k<100;k++)
        {
            for(qint16 i=0;i<SeaskyPortNum;i++)
            {
                this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vName[i] =
                settingsModule.value(QString("vNameLineTx:(%1:%2)").arg(k+1).arg(i+1),
                                     QString("Name:%1").arg(i+1)).toString();
                this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vUnit[i] =
                settingsModule.value(QString("vUnitLineTx:(%1:%2)").arg(k+1).arg(i+1),
                                     QString("Unit:%1").arg(i+1)).toString();
                this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vQString[i] =
                settingsModule.value(QString("vQStringLineTx:(%1:%2)").arg(k+1).arg(i+1),
                                     QString("0").arg(i+1)).toString();
                this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vFloat[i] =
                settingsModule.value(QString("vFloatLineTx:(%1:%2)").arg(k+1).arg(i+1),
                                     QString("0").arg(i+1)).toFloat();
            }
            this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vEquipmentType =
                settingsModule.value(QString("vEquipmentTypeLineTx:%1").arg(k+1),
                                     QString("%1").arg(0X0001)).toUInt();
            this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vEquipmentId =
                settingsModule.value(QString("vEquipmentIdLineTx:%1").arg(k+1),
                                     QString("%1").arg(0X0001)).toUInt();
            this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vDataId =
                settingsModule.value(QString("vDataIdLineTx:%1").arg(k+1),
                                     QString("%1").arg(0X00FF)).toUInt();
            this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[k].vDataLen =
                settingsModule.value(QString("vDataLenLineTx:%1").arg(k+1),
                                     QString("0")).toInt();
        }



        settingsModule.endGroup();
        vPortShow();
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
        settingsModule.setValue(QString("vEquipmentTypeTx"),
                                this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentType);
        settingsModule.setValue(QString("vEquipmentIdTx"),
                                this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vEquipmentId);
        settingsModule.setValue(QString("vDataIdTx"),
                                this->vSerialCtr.vSeaskyPortCtr.vTxSeasky.vDataId);
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
        settingsModule.setValue(QString("vEquipmentTypeRx"),
                                this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vEquipmentType);
        settingsModule.setValue(QString("vEquipmentIdRx"),
                                this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vEquipmentId);
        settingsModule.setValue(QString("vDataIdRx"),
                                this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataId);
        settingsModule.setValue(QString("vDataLenRx"),
                                this->vSerialCtr.vSeaskyPortCtr.vRxSeasky.vDataLen);
        for(uint8_t j=0;j<100;j++)
        {
            for(qint16 i=0;i<SeaskyPortNum;i++)
            {
                settingsModule.setValue(QString("vNameLineTx:(%1:%2)").arg(j+1).arg(i+1),
                                        this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vName[i]);
                settingsModule.setValue(QString("vUnitLineTx:(%1:%2)").arg(j+1).arg(i+1),
                                        this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vUnit[i]);
                settingsModule.setValue(QString("vQStringLineTx:(%1:%2)").arg(j+1).arg(i+1),
                                        this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vQString[i]);
                settingsModule.setValue(QString("vFloatLineTx:(%1:%2)").arg(j+1).arg(i+1),
                                        this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vFloat[i]);
            }
            settingsModule.setValue(QString("vEquipmentTypeLineTx:%1").arg(j+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vEquipmentType);
            settingsModule.setValue(QString("vEquipmentIdLineTx:%1").arg(j+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vEquipmentId);
            settingsModule.setValue(QString("vDataIdLineTx:%1").arg(j+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vDataId);
            settingsModule.setValue(QString("vDataLenLineTx:%1").arg(j+1),
                                    this->vSerialCtr.vSeaskyPortCtr.vTxSeaskyLine[j].vDataLen);
        }
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
        case 3:
        {

        }break;
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
           qint32                vFindCom = -1;
           QList<QList<QString>> vlist;
           CSVHelper  vCSVHelper_t(this);
           vlist = vCSVHelper_t.ReadCSVtoData(str);
           for(uint8_t i=0;i<vlist[0].size();i++)
           {
               if(FindSerialCommand==QString(vlist[0].at(i)))
               {
                   //记录特征名称值
                   vFindCom = i;
               }
           }
           if(vFindCom>=0)
           {
               for(int i = 1;i<vlist.size();i++)
               {
                   if(i-1<SerialMutipSendNum)
                   {
                       LineEditData[i-1] = vlist[i][vFindCom].toUtf8();
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
bool MainWindow::eventFilter(QObject *object, QEvent *event)//步骤二
{
    if (object == this->vLinuxTextEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *e = static_cast <QKeyEvent *> (event);
        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)//步骤三
        {
            emit vWriteData(this->vLinuxTextEdit->toPlainText()
                            .toUtf8()+"\r\n");
            this->vLinuxTextEdit->clear();
            return true;
        }
    }
    return QObject::eventFilter(object, event);
}
void MainWindow::vServerLinuxCfg(void)
{
//    scrollAreaWidgetLinux
    /*多LineEdit配置*/
    QVBoxLayout     * vQVBoxLayout =
            new QVBoxLayout(ui->scrollAreaWidgetLinux);
    QPushButton     * vpushButton,
                    * vPathButton;
    vQLineEditHex   * vlineEdit;
    QCheckBox       * vcheckBox;
    QTextEdit   * vNtextEdit;
    QWidget     * vQWidget1     =
            new QWidget(ui->scrollAreaWidgetLinux);
    QHBoxLayout * vQHBoxLayout1 =
            new QHBoxLayout(vQWidget1);
    vQHBoxLayout1->setContentsMargins(0,0,0,0);
    vNtextEdit    = new QTextEdit;
    vpushButton   =
            new QPushButton(vQWidget1);
    vNtextEdit->setMinimumSize(0,48);
    vNtextEdit->setMaximumSize(16777215,48);
    vNtextEdit->setStyleSheet("background-color:#FFFFFF;"
                              "color:#FF4500;");
    vpushButton->setMinimumSize(48,48);
    vpushButton->setMaximumSize(48,48);
    vQHBoxLayout1->addWidget(vNtextEdit);
    vQHBoxLayout1->addWidget(vpushButton);
    vQWidget1->setLayout(vQHBoxLayout1);
    vQVBoxLayout->addWidget(vQWidget1);
    vNtextEdit->installEventFilter(this);
    this->vLinuxTextEdit = vNtextEdit;
    connect(this,&MainWindow::vWriteData,
            &this->vSerialCtr.vSerial,
            &vSerialPort::vWriteData,
            Qt::QueuedConnection);
    connect(vpushButton,&QPushButton::released,[=]()
    {
        emit vWriteData(vNtextEdit->toPlainText().toUtf8()+"\r\n");
    });
    this->vLinuxTextEdit->setStyleSheet("background-color:#FFFFFF;"
                                        "color:#FF4500;");

    vLinuxData = new  QByteArray[SerialLinuxNum];
    /*-------------------导入命令配置-------------------*/
    for(qint16 i=0;i<SerialLinuxNum;i++)
    {
        QWidget   * vQWidget     =
                new QWidget(ui->scrollAreaWidgetLinux);
        QHBoxLayout * vQHBoxLayout =
                new QHBoxLayout(vQWidget);
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vlineEdit   = new vQLineEditHex(vQWidget);
        vpushButton = new QPushButton(QString::number(i+1,10),vQWidget);
        vlineEdit->setMinimumSize(0,24);
        vlineEdit->setMaximumSize(16777215,24);
        vlineEdit->setStyleSheet("background-color:#FFFFFF;"
                                 "color:#FF4500;");
        //该地址是当前发送配置地址，可以更改
        vpushButton->setMinimumSize(48,24);
        vpushButton->setMaximumSize(48,24);
        vQHBoxLayout->addWidget(vlineEdit);
        vQHBoxLayout->addWidget(vpushButton);
        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);
        connect(vpushButton,&QPushButton::released,[=]()
        {
            QByteArray str3;
            vLinuxData[i].replace("\\t","\t")
                         .replace("\\r","\r")
                         .replace("\\n","\n").toHex();
            emit vWriteData(vLinuxData[i]
                            .replace("\\t","\t")
                            .replace("\\r","\r")
                            .replace("\\n","\n"));
        });
        connect(vlineEdit,&vQLineEditHex::textChanged,[=]()
        {
            vLinuxData[i]=vlineEdit->text().toUtf8();
        });
        connect(this,&MainWindow::vLinuxShow,[=](){
            vlineEdit->setText(vLinuxData[i]);
        });

    }
}
/*Tcp初始化*/
void MainWindow::vTcpServerInit(void)
{
    connect(ui->vServerSave,&QPushButton::released,
            this,&MainWindow::vSaveTcpRxText);
    /*配置编码格式*/
    ui->comboBoxServer6->addItem("NoCode",QSerialPort::Parity(SerialCodeNo));
    ui->comboBoxServer6->addItem("UTF8",QSerialPort::Parity(SerialCodeUtf8));
    ui->comboBoxServer6->addItem("UTF16",QSerialPort::Parity(SerialCodeUtf16));
    ui->comboBoxServer6->addItem("GB18030",QSerialPort::Parity(SerialCodeGB18030));
    ui->comboBoxServer6->addItem("ISO8859",QSerialPort::Parity(SerialCodeISO8859));
    ui->comboBoxServer6->addItem("BIG5",QSerialPort::Parity(SerialCodeBig5));
    ui->comboBoxServer6->addItem("Shift-JIS",QSerialPort::Parity(SerialShiftJIS));
    /*编码格式*/
    this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpCodeMode =
            vSerialCodeModeEnum(ui->comboBoxServer6->currentData().toInt());
    connect(ui->comboBoxServer6,&QComboBox::currentTextChanged,[=](){
        this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpCodeMode =
                vSerialCodeModeEnum(ui->comboBoxServer6->currentData().toInt());
    });
    /*配置地址*/
    connect(&this->vServerTcp.vServerTcpCtr.vServerTcp,&vQTcpServer::vRxBuffChanged,
            [=](QByteArray * arr){
                if(arr!=nullptr)
                {
                    ui->plainTextTcpServer->SetShowBuffAddr(arr);
                    ui->plainTextTcpServer->TimerStart();
                }
                else
                {
                    ui->plainTextTcpServer->TimerStop();
                    ui->plainTextTcpServer->SetShowBuffAddr(nullptr);
                    ui->plainTextTcpServer->clear();
                }
            });
    ui->plainTextTcpServer->setHexEnableAddr(&this->vServerTcp.vServerTcpCtr.vServerTcp.vRxHexEnable);
    ui->plainTextTcpServer->setTimerCfg(vTimerRxPlainServer);
    ui->textEdit->setHexEnableAddr(&this->vServerTcp.vServerTcpCtr.vServerTcp.vTxHexEnable);
    //发送hex模式切换
    connect(this,
            &MainWindow::txServerHexChanged,
            ui->textEdit,
            &vQTextEdit::hexEnableChanged);
    /*hex格式是否使能*/
    connect(this,
            &MainWindow::vhexRxShowOne,
            ui->plainTextTcpServer,
            &vPlainTextEdit::hexEnableChanged);
    /*hex格式是否使能*/
    connect(ui->vSreverRxHex,&QCheckBox::released,[=]{
        this->vServerTcp.vServerTcpCtr.vServerTcp.vRxHexEnable = ui->vSreverRxHex->isChecked();
        emit vhexRxShowOne();
    });
    connect(ui->vTcpServerTxHex,&QCheckBox::released,[=]{
        this->vServerTcp.vServerTcpCtr.vServerTcp.vTxHexEnable = ui->vTcpServerTxHex->isChecked();
        emit txServerHexChanged();
    });
    //接收清除
    connect(ui->vServerClear,&QPushButton::released,[=]()
    {
        if(ui->plainTextTcpServer->vShowBuff!=nullptr)
        {
            ui->plainTextTcpServer->clearBuff();
            ui->plainTextTcpServer->TimerStart();
        }
    });
    //绑定发送清除
    connect(ui->vTcpServerTx2,&QPushButton::released,[=]()
    {
        switch(ui->tabWidgetTcp->currentIndex())
        {
            case 0:{
                ui->textEdit->TextTxBuff.clear();
                emit txServerHexChanged();
            };break;
            case 1:{
                for(int i=0;i<TcpServerMutipSendNum;i++)
                {
                    vTcpLineEditData[i].clear();
                }
                emit txServerHexChanged();
            };break;
        }
    });
    //连接到端口更新连接端口
    void (MainWindow::*vTcpListCfg1)()=&MainWindow::vTcpListCfg;
    connect(&this->vServerTcp.vServerTcpCtr.vServerTcp,&vQTcpServer::vNewSocket,
            this,vTcpListCfg1);
    //端口改变
    void (QComboBox::*vServer5Changed)(int)=&QComboBox::activated;
    void (MainWindow::*vTcpListCfg2)(int)=&MainWindow::vTcpListCfg;
    connect(ui->comboBoxServer5,vServer5Changed,
            this,vTcpListCfg2);
    //发送窗口
    connect(ui->vTcpServerTx1,&QPushButton::released,
            &this->vServerTcp.vServerTcpCtr,&vServerQObject::vTxTimeOut);
    //打开TCP
    connect(ui->vServerOpen,&QPushButton::released,[=]()
    {
        if(ui->vServerOpen->isChecked())
        {
            this->vServerTcp.vServerTcpCtr.vServerTcp.vSetPort(ui->comboBoxServer1->currentData().toInt());
            if(this->vServerTcp.vServerTcpCtr.vServerTcp.vOpenServer())
            {
                ui->vServerOpen->setChecked(true);
            }
            else
            {
                ui->vServerOpen->setChecked(false);
            }
        }
        else
        {
            this->vServerTcp.vServerTcpCtr.vServerTcp.vCloseServer();
        }
    });

    /*端口显示窗口*/
    ui->comboBoxServer1->addItem(QString::fromLocal8Bit("添加端口号"),0);
    //端口改变
    void (QComboBox::*vChanged)(int)=&QComboBox::activated;
    connect(ui->comboBoxServer1,vChanged,
            this,&MainWindow::vTcpServerChanged);

    connect(ui->comboBoxServer5,vChanged,
            [=](int index){
                this->vServerTcp.vServerTcpCtr.vServerTcp.vConfigBuffAddr(index);
            });
    //加载多条显示框
    vTxInfosInit(TcpServerMutipSendNum);

    /*Tcp发送相关的定时器*/
    this->vServerTcp.vServerTcpCtr.vTxTimerCntSet(ui->vTcpServerTimCnt->text().toInt());
    connect(ui->vTcpServerTimCnt,&QSpinBox::textChanged,[=](){
        this->vServerTcp.vServerTcpCtr.vTxTimerCntSet(ui->vTcpServerTimCnt->text().toInt());
    });
    //定时器控制
    connect(ui->vTcpServerTimer,&QCheckBox::released,
            this,&MainWindow::vTcpTimerCfg);
    connect(ui->vTcpServerTxs,&QCheckBox::released,
            this,&MainWindow::vTcpTxModeCfg);
    ui->comboBoxTcpL->addItem("[ \\r\\n ]\t(CRLF)","\r\n");
    ui->comboBoxTcpL->addItem("[ \\n ]\t(LF)","\n");
    ui->comboBoxTcpL->addItem("[ } ]\t(JSON)","}");
    this->vServerTcp.vServerTcpCtr.vServerTcp.vSetRxStampStr(ui->comboBoxTcpL->currentData().toString().toUtf8());
    connect(ui->comboBoxTcpL,&QComboBox::currentTextChanged,[=]()
    {
        this->vServerTcp.vServerTcpCtr.vServerTcp.vSetRxStampStr(ui->comboBoxTcpL->currentData().toString().toUtf8());
    });
    /*配置接收时间戳处理*/
    connect(ui->vSreverRxTim,&QCheckBox::released,[=](){
        this->vServerTcp.vServerTcpCtr.vServerTcp.rxTimeStamp =ui->vSreverRxTim->isChecked();
    });

    //发送加入换行符
    connect(ui->vTcpServeradd,&QCheckBox::released,[=]()
    {
        vTxTcpStampCfg();
    });

}
void MainWindow::vSaveTcpRxText(void)
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
        out<<ui->plainTextTcpServer->toPlainText();
    }
}
void MainWindow::vTcpTxModeCfg(void)
{
    if(ui->vTcpServerTxs->isChecked())
    {
        this->vServerTcp.vServerTcpCtr.vTcpTxCfg = vTcpMul;
    }
    else
    {
        this->vServerTcp.vServerTcpCtr.vTcpTxCfg = vTcpOrd;
    }
}
void MainWindow::vTcpTimerCfg(void)
{
    if(ui->vTcpServerTimer->isChecked())
    {
        this->vServerTcp.vServerTcpCtr.vTxTimerStop();
        this->vServerTcp.vServerTcpCtr.vTxTimerStart();
    }
    else
    {
        this->vServerTcp.vServerTcpCtr.vTxTimerStop();
    }
}
void MainWindow::vTcpWrite(const QByteArray &str)
{
    this->vServerTcp.vServerTcpCtr.vServerTcp.vWriteStr(str);
}
void MainWindow::vTcpListCfg(int index)
{
    int checkIndex = index;
    {
        ui->comboBoxServer2->setText(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[checkIndex]->localAddress().toString());
        ui->comboBoxServer3->setText(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[checkIndex]->peerAddress().toString());
        ui->comboBoxServer4->setText(QString("%1").arg(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[checkIndex]->peerPort()));
    }
    this->vServerTcp.vServerTcpCtr.vServerTcp.vConfigBuffAddr(checkIndex);
}
void MainWindow::vTcpListCfg(void)
{
    if(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpCnt>0)
    {
        quint16 currentData    = 0;
        int     checkIndex     = 0;
        int     lastIndex      = 0;
        bool    isConnectState = false;
        QAbstractSocket::SocketState vSocketState;
        //需要记录上次选中的端口,不随意自动切换
        currentData = quint16(ui->comboBoxServer5->currentData().toUInt());
        lastIndex   = ui->comboBoxServer5->currentIndex();
        checkIndex  = this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpCnt-1;
        //清除后、重新加载
        ui->comboBoxServer5->clear();
        for(int i=0;i<this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpCnt;i++)
        {
            vSocketState = this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[i]->state();
            if(vSocketState == QAbstractSocket::ConnectedState)
            {
                isConnectState = true;
            }
            else
            {
                isConnectState = false;
            }
            ui->comboBoxServer5->addItem(QString("%1[%2]")
                .arg(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpListCom.at(i))
                .arg(isConnectState),
                quint16(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpListCom.at(i)));
            if(lastIndex>=0)
            {
                //如果接口还可用
                if(bool(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[lastIndex]->peerPort()))
                {
                    if(currentData == this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpListCom.at(i))
                    {
                        checkIndex = i;//如果原来是拥有的，后续需要继续选定
                    }
                }
            }
        }
        {
            ui->comboBoxServer2->setText(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[checkIndex]->localAddress().toString());
            ui->comboBoxServer3->setText(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[checkIndex]->peerAddress().toString());
            ui->comboBoxServer4->setText(QString("%1").arg(this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpList[checkIndex]->peerPort()));
        }
        ui->comboBoxServer5->setCurrentIndex(checkIndex);
        this->vServerTcp.vServerTcpCtr.vServerTcp.vConfigBuffAddr(checkIndex);
        ui->plainTextTcpServer->TimerStart();
    }
}
void MainWindow::vImportTcpText(QString str)
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
           qint32                vFindCom = -1;
           QList<QList<QString>> vlist;
           CSVHelper  vCSVHelper_t(this);
           vlist = vCSVHelper_t.ReadCSVtoData(str);
           for(uint8_t i=0;i<vlist[0].size();i++)
           {
               if(FindTcpCommand==QString(vlist[0].at(i)))
               {
                   //记录特征名称值
                   vFindCom = i;
               }
           }
           if(vFindCom>=0)
           {
               for(int i = 1;i<vlist.size();i++)
               {
                   if(i-1<TcpServerMutipSendNum)
                   {
                       vTcpLineEditData[i-1] = vlist[i][vFindCom].toUtf8();
                       vTcpHexEnableCfg();
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
/*初始化多条窗口*/
void MainWindow::vTxInfosInit(qint32 MultPleNum)
{
    /*多LineEdit配置*/
    this->TcpMaxCnt = MultPleNum;
    QVBoxLayout     * vQVBoxLayout =
            new QVBoxLayout(ui->scrollAreaWidgetContents_5);
    QPushButton     * vpushButton,
                    * vPathButton;
    vQLineEditHex   * vlineEdit;
    QCheckBox       * vcheckBox;
    QLineEdit       * vQlineEdit;
    //分配依赖的地址
    vTcpLineEditData = new QByteArray[this->TcpMaxCnt];
    vTcpLineEditMap =  new QMap<qint32,bool*>;
    //原始数据地址共享,该地址只能在此分配
    this->vServerTcp.vServerTcpCtr.vAddrSet(
                                  MultPleNum,
                                  &vTcpLineEditData[0],
                                  vTcpLineEditMap);
    this->vServerTcp.vServerTcpCtr.vTxAddrSet(
                    &ui->textEdit->TextTxBuff);
    //添加一个自动导入命令的操作支持
    /*-------------------导入命令配置-------------------*/
    QWidget     * vQWidget1   =
            new QWidget(ui->scrollAreaWidgetContents_5);
    QHBoxLayout * vQHBoxLayout1 =
            new QHBoxLayout(vQWidget1);
    vQHBoxLayout1->setContentsMargins(0,0,0,0);
    vPathButton   = new QPushButton(vQWidget1);
    vQlineEdit    = new QLineEdit(vQWidget1);
    vpushButton   =
            new QPushButton(QString::fromLocal8Bit("导入"),vQWidget1);
    vPathButton->setText("./");
    vPathButton->setStyleSheet("color:#FFFFFF;font-weight: bold;");
    vPathButton->setMinimumSize(24,24);
    vPathButton->setMaximumSize(24,24);
    vQlineEdit->setMinimumSize(0,24);
    vQlineEdit->setMaximumSize(16777215,24);
    vQlineEdit->setStyleSheet("background-color:#FFFFFF;"
                              "color:#FF4500;");
    vpushButton->setMinimumSize(48,24);
    vpushButton->setMaximumSize(48,24);
    vQHBoxLayout1->addWidget(vPathButton);
    vQHBoxLayout1->addWidget(vQlineEdit);
    vQHBoxLayout1->addWidget(vpushButton);
    vQWidget1->setLayout(vQHBoxLayout1);
    vQVBoxLayout->addWidget(vQWidget1);
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
    connect(vpushButton,&QPushButton::released,[=]()
    {
        vImportTcpText(vQlineEdit->text());
    });
    /*-------------------导入命令配置-------------------*/
    for(qint16 i=0;i<this->TcpMaxCnt;i++)
    {
        QWidget     * vQWidget     =
                new QWidget(ui->scrollAreaWidgetContents_5);
        QHBoxLayout * vQHBoxLayout =
                new QHBoxLayout(vQWidget);
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vcheckBox   = new QCheckBox(vQWidget);
        vlineEdit   = new vQLineEditHex(vQWidget);
        vpushButton = new QPushButton(QString::number(i+1,10),vQWidget);

        vcheckBox->setMinimumSize(24,24);
        vcheckBox->setMaximumSize(24,24);
        vlineEdit->setMinimumSize(0,24);
        vlineEdit->setMaximumSize(16777215,24);
        vlineEdit->setStyleSheet("background-color:#FFFFFF;"
                                 "color:#FF4500;");
        //设置窗口ID
        vlineEdit->QLineId = i+1;
        //初始化是否被选中
        vlineEdit->isTxEnable =
                vcheckBox->isChecked();
        //分配地址
        vlineEdit->setTextTxBuffAddr(&vTcpLineEditData[i]);
        //该地址是当前发送配置地址，可以更改
        vpushButton->setMinimumSize(48,24);
        vpushButton->setMaximumSize(48,24);
        vQHBoxLayout->addWidget(vcheckBox);
        vQHBoxLayout->addWidget(vlineEdit);
        vQHBoxLayout->addWidget(vpushButton);
        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);
        //hex格式共享，所有控件建议只读
        vlineEdit->setHexEnableAddr(&this->vServerTcp.vServerTcpCtr.vServerTcp.vTxHexEnable);
        /*发送端hex配置,本地如果hex格式标志改变，使用信号与槽更新每个控件的显示*/
        connect(this,
                &MainWindow::txServerHexChanged,
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
        vTcpLineEditMap->insert(i+1,&vlineEdit->isTxEnable);
        connect(this,&MainWindow::vMapTcpUpdata,[=]()
        {
            if(vTcpLineEditData[i].size()>0)
            {
                vcheckBox->setChecked(*vTcpLineEditMap->find(i+1).value());
            }
            else
            {
                *vTcpLineEditMap->find(i+1).value() = false;
            }
        });
    }
    ui->scrollAreaWidgetContents_5->setLayout(vQVBoxLayout);
    //触发LineEdit发送
    connect(this,&MainWindow::lineEditTxOne,
            &this->vServerTcp.vServerTcpCtr,
            &vServerQObject::vLineEditTxOne,
            Qt::QueuedConnection);
}
//刷新TxHexEnable
void MainWindow::vTcpHexEnableCfg(void)
{
    if(ui->vTcpServerTxHex->isChecked())
    {
        this->vServerTcp.vServerTcpCtr.vServerTcp.vTxHexEnable = true;
    }
    else
    {
        this->vServerTcp.vServerTcpCtr.vServerTcp.vTxHexEnable = false;
    }
    emit txServerHexChanged();
}
//发送换行符控制
void MainWindow::vTxTcpStampCfg(void)
{
    if(ui->vTcpServeradd->isChecked())
    {
        this->vServerTcp.vServerTcpCtr.LineFeed=QByteArray("\n");
    }
    else
    {
        this->vServerTcp.vServerTcpCtr.LineFeed=QByteArray("");
    }
}
//重新加载部分参数
void MainWindow::vTcpCfgUpdata(void)
{
    //设置编码格式
    this->vServerTcp.vServerTcpCtr.vServerTcp.vTcpCodeMode =
            vSerialCodeModeEnum(ui->comboBoxServer6->currentData().toInt());
    //接收换行方式
    this->vServerTcp.vServerTcpCtr.vServerTcp.vSetRxStampStr(ui->comboBoxTcpL->currentData().toString().toUtf8());
    //接收HEX格式使能
    this->vServerTcp.vServerTcpCtr.vServerTcp.vRxHexEnable = ui->vSreverRxHex->isChecked();
    //接收换行使能
    this->vServerTcp.vServerTcpCtr.vServerTcp.rxTimeStamp =ui->vSreverRxTim->isChecked();
    //发送模式设置
    vTcpTxModeCfg();
    //发送HEX格式使能
    vTcpHexEnableCfg();
    //发送定时器
    this->vServerTcp.vServerTcpCtr.vTxTimerCntSet(ui->vTcpServerTimCnt->text().toInt());
    emit vMapTcpUpdata();
    vTxTcpStampCfg();
}
void MainWindow::vTcpServerChanged(qint16 index_t)
{
    if(ui->comboBoxServer1->currentText()==QString::fromLocal8Bit("添加端口号"))
    {
        bool ok,nohave;
        int i;
        QString str;
        ok = false;
        nohave = false;
        str =
        QInputDialog::getText(this,QString::fromLocal8Bit("添加端口号"),
                      QString::fromLocal8Bit("新的端口名称"),
                      QLineEdit::Normal,"0",
                      &ok,Qt::WindowCloseButtonHint);
        for(i=0;i<ui->comboBoxServer1->count();i++)
        {
            if(ok)
            {
                if(ui->comboBoxServer1->itemText(i)==str)//如果已经有了
                {
                    nohave = false;
                    break;
                }
            }
        }
        if(i>=ui->comboBoxServer1->count()){nohave = true;}
        if(nohave&ok)
        {
            ui->comboBoxServer1->setItemText(ui->comboBoxServer1->count()-1,str);
            ui->comboBoxServer1->setItemData(ui->comboBoxServer1->count()-1,str.toInt());
            ui->comboBoxServer1->addItem(QString::fromLocal8Bit("添加端口号"),0);
        }
        else if(!nohave)
        {
            doWarning(QString::fromLocal8Bit("已添加该端口号"));
        }
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


