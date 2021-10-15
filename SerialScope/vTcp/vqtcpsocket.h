/*
 ************************************** Copyright ******************************
 * @FileName     :  vqtcpsocket.h
 * @Author       :  Seasky.Liu
 * @Date         :  2021/8/30
 * @Time         :  19:28
 * @Email        :  xxx.
 ******************************************************************************
 */

#ifndef VQTCPSOCKET_H
#define VQTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class vQTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit    vQTcpSocket(QTcpSocket *parent = nullptr);
    void        vOpenSocket(void); //打开链接
    void        vCloseSocket(void);//关闭链接
    void        vSetPort(qint16 port_t);
    void        vConnected(void);
    void        vDisConnected(void);
private:
    quint16 vport;
    bool    vconnect=false,vconnected=false;

signals:

};

#endif // VQTCPSOCKET_H
