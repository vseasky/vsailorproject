#ifndef VTCPCTR_H
#define VTCPCTR_H

#include <QObject>
#include <vserverthread.h>


typedef enum
{
   vTcpCtrOrd=0,//普通发送模式
   vTcpCtrMul  ,  //多条连续发送模式
   vTcpCtrAgr  ,  //按协议发送模式
}vTcpCtrTxMode;

class vTcpCtr : public QObject
{
    Q_OBJECT
public:
    explicit vTcpCtr(QObject *parent = nullptr);
    ~vTcpCtr(void);
    vServerQObject vServerTcpCtr;
    vServerThread  vServerThreadCtr;
signals:

};

#endif // VTCPCTR_H
