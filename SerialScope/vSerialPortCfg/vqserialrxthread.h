#ifndef VQSERIALRXTHREAD_H
#define VQSERIALRXTHREAD_H

#include <QObject>
#include <QThread>
#include <QDateTime>
#include <QSerialPort>
#include <vserialport.h>

#include <QDebug>

class vQObjectRx : public QObject
{
    Q_OBJECT
public:
    explicit vQObjectRx(QObject *parent = nullptr);
    ~vQObjectRx(void);
    /*------------串口基础类，必须初始化------------*/
    vSerialPort   *vSerial;
    void vSerialAddrSet(vSerialPort   *vSerial_t)
    {
        vSerial = vSerial_t;
    }
    /*------------串口基础类，必须初始化------------*/
    void vConnectRx(void);
    void vDisConnectRx(void);

public slots:
    void vReadSlot(void);
private:
signals:
    void vRxShow(const QByteArray &str);
};

class vQserialRxThread : public QThread
{
    Q_OBJECT
public:
    explicit vQserialRxThread(QObject *parent = nullptr);
public:
    void run(void)
    {
        qDebug()<<"main tid:vQObjectRx run"<< QThread::currentThreadId();
        exec();
    }
signals:
public slots:
};

#endif // VQSERIALRXTHREAD_H
