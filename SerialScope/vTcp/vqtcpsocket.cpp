/*
 ************************************** Copyright ******************************
 * @FileName     :  vqtcpsocket.cpp
 * @Author       :  Seasky.Liu
 * @Date         :  2021/8/30
 * @Time         :  19:28
 * @Email        :  xxx.
 ******************************************************************************
 */
#include "vqtcpsocket.h"
#include <QHostAddress>
#include <QThread>
#include <QDebug>
vQTcpSocket::vQTcpSocket(QTcpSocket *parent) : QTcpSocket(parent)
{

}
void vQTcpSocket::vOpenSocket(void)
{

}
void vQTcpSocket::vCloseSocket(void)
{

}
void vQTcpSocket::vConnected(void)
{

}
void vQTcpSocket::vDisConnected(void)
{
    qDebug()<<"DisConnected"<<endl;
}
void vQTcpSocket::vSetPort(qint16 port_t)
{
    this->vport = port_t;
}
