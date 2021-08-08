#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <vserialcom.h>

#include <vQLineEditHex/vqlineedithex.h>
#include <vSeaskyPort/vseaskyport.h>
#include <vcafesclient.h>
#include <QMap>
#include <QMapIterator>
#include <vopenglwidget.h>
#include <QString>

/*后续考虑加入Linux的支持*/
#define WinDownVersion

#ifndef WinDownVersion
#define LinuxVersion
#else
#include <windows.h>
#include <dbt.h>
#include <devguid.h>
#include <setupapi.h>
#include <initguid.h>
#endif


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //LineEdit数据地址，以及选中标志
    vOpenGlWidget      vOpenGlWidget;
    QByteArray         * LineEditData;  //提供数据原始地址 使用NEW创建
    QMap<qint32,bool*> * LineEditMap;   //提供可查询的是否发送标志
    qint32             MultPleMaxCnt;
    //基础功能的串口类
    vSerialCom    vSerialCtr;
    bool          rxModeCfg = false;//是否使能协议
    bool          txModeCfg = false;//是否使能协议
    //电脑信息
    vCafesClient  vCafes;

    QTimer vRxTxInfoTimer;
    /*----------------串口插拔检测-----------------*/
#ifdef WinDownVersion
    void vWindownInit(void);
#else
    void vLinuxInit(void);
#endif
    /*-------------------------------------------*/
    /*初始化操作主要包含信号与槽的建立，以及信号与槽的逻辑*/
    void vDependenceAddr(void); //0.初始化相关的地址
    void vShowInit(void);       //1.加载自定义图形
    void vLineEditShowInit(qint32 MultPleNum);
    void vInitDetection(void);  //2.串口检测相关的初始化,检测串口设备
    void vUpdateComInfo(void);  //3.串口设置内容初始化，波特率等
    void vUpdateSerial(void);   //4.初始化串口设备更新支持
    void vInitSerialTx(void);   //5.加载信息发送配置
    void vInitSerialRx(void);   //6.加载信息接收配置
    void vInfoChangedInit(void);    //7.串口信息更新，只需要初始化一次
    void vInitSeasky(void);     //8.串口协议初始化
    void vInitControl(void);    //9.加载控件列表->8个按钮
    void vShowTimerCfg();       //10.初始化定时器
    /*-------------------------------------------*/
    void readSerialChange(void);//读取选中串口
    void SerialOpen(void);      //打开串口
    void SerialClose(void);     //关闭串口
    /*--------------界面配置恢复相关----------------*/
    void vReadSettings(void);       //启动时读取设置
    void vWriteSettings(void);      //关闭时保存设置
    /*-------------------------------------------*/
    void vControlCmd(qint16 cmdID); //自定义控件按钮,按下会响应

    void vStatusbarCfg(void);       //设置显示接收、发送统计

    //获取路径中需要的文件列表
    QStringList vGetFilrName(const QString &strPath,
                         const QStringList &filters);

public slots:
    /*----------------保存显示数据------------------*/
    void vSaveRxText(void);         //保存接收窗口
    void vSaveRxSeaskyText(void);   //保存协议接收窗口
    /*------------------刷新配置-------------------*/
    void vRxSlotChanged(void);        //刷新接收槽函数连接
    void vTxSlotChanged(void);        //刷新发送槽函数连接
    void vModuleChanged(qint16 index);//刷新Seasky协议模块
    void vRxTimerStampChanged(void);  //刷新接收时间戳使能
    void showRxHead(void);            //刷新协议RX显示
    void showTxHead(void);            //刷新协议TX显示
    void vPortShow(void);             //刷新协议显示
    void vRxHexEnableCfg(void);       //刷新RxHexEnable
    void vTxHexEnableCfg(void);       //刷新TxHexEnable
    void vTxModeCfg(void);            //发送模式切换处理
    void vTxStampCfg(void);           //发送换行符控制
    void vTxModeTimerCfg(void);       //发送的定时器控制
    void vUpdateShow(void);           //同步窗口和变量数据
    /*-------------------------------------------*/
    void vShowModule(qint16 index);   //刷新协议模块
    void vModuleAddItem(void);        //添加SEASKY协议模块
    void vSaveModule(void);           //保存SEASKY协议模块数据
    /*-------------------------------------------*/
    void vImportLineText(QString str);//导入Csv数据配置
    /*-------------------------------------------*/
    void vTabTimerCfg(void);

    void vTxError(void);

    void vSerialStatusCheck(void);
signals:
    void plaintextEditShowOne(void);
    void lineEditTxOne(qint32 num);
    void txHexEnableChanged(void);
    void vOpenSerial(bool & isOpen);
    void vCloseSerial(void);
protected:
    void closeEvent(QCloseEvent *event);
    void doCritical(const QString &str);
    void doWarning(const QString &str);
    void doHelp(void);
    void doAbout(void);
    /*----------------串口插拔检测-----------------*/
#ifdef WinDownVersion
    bool nativeEvent(const QByteArray & eventType, void * message, long*result);
#else
    //Linux
#endif
    /*----------------串口插拔检测-----------------*/
private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
