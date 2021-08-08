#ifndef VSERIALCOM_H
#define VSERIALCOM_H

#include <QObject>
#include <vqserialrxthread.h>
#include <vqserialtxthread.h>
#include <vseaskyport.h>
//创建串口应用
class vSerialCom : public QObject
{
    Q_OBJECT
public:
    explicit vSerialCom(QObject *parent = nullptr);
    ~vSerialCom(void);
    vSerialPort    vSerial;
    vSerialThread  vComThread;
    /*将自定义的串口类实体化*/
    vQObjectTx   vQObjectTxCtr;
    vQserialTxThread vTxThread;
    vQObjectRx   vQObjectRxCtr;
    vQserialRxThread vRxThread;
    /*Seasky协议*/
    vSeaskyPort        vSeaskyPortCtr;
    vSeaskyPortQThread vSeaskyThread;
signals:

};

#endif // VSERIALCOM_H
