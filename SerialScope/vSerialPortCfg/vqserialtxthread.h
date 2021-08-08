#ifndef VQSERIALTXTHREAD_H
#define VQSERIALTXTHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <vserialport.h>

#include <QDebug>

typedef enum
{
   SerialOrd=0,//普通发送模式
   SerialMul,  //多条连续发送模式
   SerialAgr,  //按协议发送模式
}SerialTxMode;
class vQObjectTx : public QObject
{
    Q_OBJECT
public:
    explicit vQObjectTx(QObject *parent = nullptr);
    ~vQObjectTx(void);
    /*------------串口基础类，必须初始化------------*/
    vSerialPort   *vSerial;
    void vSerialAddrSet(vSerialPort   *vSerial_t)
    {
        vSerial = vSerial_t;
    }
    /*------------串口基础类，必须初始化------------*/
    SerialTxMode  vSerialTxMode=SerialOrd;

    //多次发送
    bool vLineEditTxCtr(void);
    //单次发送
    void vLineEditTxOne(qint32 num);

    //涉及到切换
    void vConnectTx(void);
    void vDisConnectTx(void);

    /*定时发送配置*/
    QTimer   vQTimer;
    void vTimerSet(qint32 cnt);
    void vTimerStart(void);
    void vTimerStop(void);

    /*发送时分配到的地址*/
    QByteArray * textEditTx;//单条数据发送，外部设定
    QByteArray * lineEditTx;//多条发送 ，内部更新
    QByteArray * SeaskyTx;  //按协议发送
    QByteArray   LineFeed;  //换行符

    /*多条发送存储地址*/
    QByteArray         * LineEditData;
    QMap<qint32,bool*> * LineEditMap;
    qint32             * MultPleMaxCnt;

    qint32               timerCnt=0;
    void vTxAddrSet(QByteArray * dataAddr);
    void vAddrSet(qint32 * CntAddr,
                  QByteArray * dataAddr,
                  QMap<qint32,
                  bool*> *mapAddr);
public slots:
    void vTxTimeOut(void);

private:
    //定时发送操作
    qint32   vTimerOutTime;
signals:
    void vTxMessage(const QByteArray &str);
};

class vQserialTxThread : public QThread
{
    Q_OBJECT
public:
    explicit vQserialTxThread(QObject *parent = nullptr);
public:
    void run(void)
    {
        qDebug()<<"main tid:vQObjectTx run"<< QThread::currentThreadId();
        exec();
    }
signals:
public slots:
};

#endif // VQSERIALTXTHREAD_H
