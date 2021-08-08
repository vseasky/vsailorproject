#ifndef VSEASKYPORT_H
#define VSEASKYPORT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QRegExpValidator>
#include <QLineEdit>
#include <QThread>
#include <QTimer>
#include <vSeaskyPort/Protocol/bsp_protocol.h>
#include <vPlainTextEdit/vplaintextedit.h>
#include <QSerialPort>
#include <vserialport.h>

#include <QDebug>
typedef struct
{
    QString * vQString; //float转str数据
    QString * vName;    //数据名称
    QString * vUnit;    //单位名称
    float   * vFloat;   //float数据

    quint16   vCmdId;   //CMD ID
    quint16   vReg;     //16位寄存器
    qint16    vDataLen; //数据长度
}vSeaskyData;

class vSeaskyPort : public QObject
{
    Q_OBJECT
public:
    explicit vSeaskyPort(QWidget *parent = nullptr);
    /*------------串口基础类，必须初始化------------*/
    vSerialPort   *vSerial;
    void vSerialAddrSet(vSerialPort   *vSerial_t)
    {
        vSerial = vSerial_t;
    }
    /*------------串口基础类，必须初始化------------*/
    void vConnectRx(void);
    void vDisConnectRx(void);
    void vConnectTx(void);
    void vDisConnectTx(void);
    QWidget        * vTxEdit;
    QWidget        * vRxEdit;
    vSeaskyData vRxSeasky;
    vSeaskyData vTxSeasky;

    QVector<float> vRxdata;
    QVariant ShowQVariant;

    QByteArray  vRxShow;
    QByteArray  vRxBuff;//数据处理缓冲，中间量
    QByteArray  vSeaskyTxBuff;
    SerialProtocol vProtocol;

    bool      vQTimerEnable;
    QTimer    vQTimer;
    qint32    timerCntSet=100;
    uint16_t rx_pos=0,thisLength=0;
    vPlainTextEdit * vPlainEdit;
    //用于协议发送计时器
    QTimer    vQTimerTx;
    qint32    vQtimerTxCnt=100;
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
    void vHeadCheck(void);
    void vUpdateShowBuff(const QString &currentTimer);
    void setQWidgetAddr(QWidget * addrTx,QWidget * addrRx);
    void setRxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF);
    void setTxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF);
    void configQWidgetRx(qint32 num);
    void configQWidgetTx(qint32 num);
public slots:
    void vSeaskyRxIRQ(void);
    void vSeaskyRxIRQ(const QByteArray &str);
    void vSeaskyTxSlot(void);
private:
    qint32  vTxNum,vRxNum;
    qint32  vTxNumUTF8,vRxNumUTF8;
public:
signals:
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
