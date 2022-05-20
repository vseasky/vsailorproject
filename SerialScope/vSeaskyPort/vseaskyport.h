#ifndef VSEASKYPORT_H
#define VSEASKYPORT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QRegExpValidator>
#include <QLineEdit>
#include <QThread>
#include <QTimer>
#include <vSeaskyPort/Protocol/bsp_protocol_class.h>
#include <vPlainTextEdit/vplaintextedit.h>
#include <QSerialPort>
#include <vserialport.h>

#include <QDebug>
typedef struct
{
    QString * vQString = nullptr; //float转str数据
    QString * vName   = nullptr;    //数据名称
    QString * vUnit   = nullptr;    //单位名称
    float   * vFloat  = nullptr;   //float数据

    /*字节偏移 4 -->> 字节大小 2*/
    quint16     vEquipmentType; //设备类型
    /*字节偏移 6 -->> 字节大小 2*/
    quint16     vEquipmentId; 	 //设备ID
    /*字节偏移 8 -->> 字节大小 data_len+2 */
    quint16     vDataId; //数据ID
    qint16      vDataLen; //数据长度
}vSeaskyData;

class vSeaskyPort : public QObject
{
    Q_OBJECT
public:
    explicit vSeaskyPort(QWidget *parent = nullptr);
    /*------------串口基础类，必须初始化------------*/
    vSerialPort   *vSerial = nullptr;
    void vSerialAddrSet(vSerialPort   *vSerial_t)
    {
        vSerial = vSerial_t;
    }
    /*------------串口基础类，必须初始化------------*/
    void vConnect(void);
    void vDisConnect(void);
    QWidget        * vTxEdit = nullptr;
    QWidget        * vRxEdit = nullptr;
    QWidget        * vTxEditScope = nullptr;
    protocol_struct  * pTxProtocolLine = new protocol_struct();
    vSeaskyData vRxSeasky;
    vSeaskyData vTxSeasky;
    vSeaskyData *vTxSeaskyLine;
    QVector<float> vRxdata;
    QVariant ShowQVariant;

    QByteArray  vRxShow;
    QByteArray  vRxBuff;//数据处理缓冲，中间量
    QByteArray  vSeaskyTxBuff;
    ComCanClass vProtocol;

    bool      vQTimerEnable;
    bool      vDataAutoTxEnable = false;
    QTimer    vQTimer;
    qint32    timerCntSet=100;
    vPlainTextEdit * vPlainEdit = nullptr;
    bool           * vLineEditTxEnable = nullptr;
    QMap<qint32,bool*> * vLineEditMap   = nullptr;
    //用于协议发送计时器
    QTimer    vQTimerTx;
    qint32    vQtimerTxCnt=100;
    qint32    MultPleMaxCnt = 100;
    qint32    timerCnt = 0;
    qint32    currIndex = 0;
    void setTimer(qint32 Cnt)
    {
        timerCntSet = Cnt;
    }
    void setVtimerTxCnt(qint32 Cnt)
    {
        vQtimerTxCnt = Cnt;
    };
    void vQTimerTxStart(void);
    void vQTimerTxStop(void);
    void setPlainEdit(vPlainTextEdit * edit);
    void timerStart(void);
    void timerStop(void);
    void vUpdateShowBuff(const QString &currentTimer);
    void setQWidgetAddr(QWidget * addrTx,QWidget * addrRx);
    void setRxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF);
    void setTxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF);
    void configQWidgetRx(qint32 num);
    void configQWidgetTx(qint32 num);
    void configQWidgetEditTx(QWidget *pQWidget,qint32 numPort,qint32 num);
    void vDataAutoTxLines(uint8_t index);
    bool vLineEditTxCtr(void);
public slots:
    void vSeaskyRxIRQ(void);
    void vSeaskyRxIRQ(const QByteArray &str);
    void vSeaskyTxSlot(void);
private:
    qint32  vTxNum,vRxNum;
    qint32  vTxNumUTF8,vRxNumUTF8;
public:
signals:
    void vQWidgetTxLineShow1(void);
    void vQWidgetTxLineShow2(void);
    void vQWidgetRxShow(void);
    void vQWidgetTxShow(void);
    void textChanged(void);
    void vInfoChanged(void);
    void showRxHead(void);
    void vSerialTx(const QByteArray & str);
    void RxScope(const QVariant &rxData);
};
class vSeaskyPortQThread : public QThread
{
    Q_OBJECT
public:
    explicit vSeaskyPortQThread(QObject *parent = nullptr);
public:
    void run(void)
    {
        qDebug()<<"main tid:vSeaskyPort run"
                <<QThread::currentThreadId();
        exec();
    }
signals:
public slots:
};
#endif // VSEASKYPORT_H
