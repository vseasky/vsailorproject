#ifndef VSERVERTHREAD_H
#define VSERVERTHREAD_H

#include <QObject>
#include <QThread>
#include <vqtcpserver.h>

#include <QDebug>
typedef enum
{
   vTcpOrd=0,//普通发送模式
   vTcpMul,  //多条连续发送模式
   vTcpAgr,  //按协议发送模式
}vTcpTxMode;
class vServerQObject : public QObject
{
    Q_OBJECT
public:
    explicit vServerQObject(QObject *parent = nullptr);
    QByteArray         * vTcpLineData   = nullptr;//提供数据原始地址 使用NEW创建
    QByteArray         * vTcpLineTxData = nullptr;//多条选中发送
    QByteArray         * vTcpTxData     = nullptr;//单条
    QMap<qint32,bool*> * vTcpLineMap    = nullptr; //提供可查询的是否发送标志
    QByteArray           LineFeed;  //换行符
    qint32               TcpMaxCnt;
    qint32               vTxTimerCnt    =100;
    qint32               timerCnt;
    QTimer               vTxTimer;

    vTcpTxMode           vTcpTxCfg    = vTcpOrd;
    //TCP支持
    vQTcpServer vServerTcp;

    void vTxTimerCntSet(qint32 index_t);
    void vTxTimerStart(void);
    void vTxTimerStop(void);

    void vTxTimeOut(void);
    bool vLineEditTxCtr(void);
    void vLineEditTxOne(qint32 num);


    void vTxAddrSet(QByteArray * dataAddr);
    void vAddrSet(qint32       Cnt,
                  QByteArray * dataAddr,
                  QMap<qint32,bool*> *mapAddr);

public slots:
    void vTimerText(void);
signals:
    void vTcpTxMsg(const QByteArray & str);
};
class vServerThread : public QThread
{
    Q_OBJECT
public:
    explicit vServerThread(QObject *parent = nullptr);

public:
    void run(void)
    {
        qDebug()<<"main tid:vServerThread run"<< QThread::currentThreadId();
        exec();
    }
signals:
public slots:
};
#endif // VSERVERTHREAD_H
