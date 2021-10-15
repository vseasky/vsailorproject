/*
 ************************************** Copyright ******************************
 * @FileName     :  vqtcpserver.cpp
 * @Author       :  Seasky.Liu
 * @Date         :  2021/8/30
 * @Time         :  19:28
 * @Email        :  xxx.
 ******************************************************************************
 */

#include "vqtcpserver.h"
#include <QHostAddress>
#include <QDateTime>
/*
 * 对于本机而言同一时间，只能向一个端口发送数据
 * 但是需要可以同时接收来自多个端口的数据
 */
//构造函数
vQTcpServer::vQTcpServer(QTcpServer *parent) : QTcpServer(parent)
{
    //监视Tcp服务器状态
    connect(this,&QTcpServer::newConnection,
            this,&vQTcpServer::vNewConnect);
}
vQTcpServer::~vQTcpServer()
{

}
bool vQTcpServer::vOpenServer(void)
{
    this->vCloseServer();
    if(!this->listen(QHostAddress::AnyIPv4,this->vServerPort))
    {
        doWarning("创建失败!");
        return false;
    }
    return true;
}
void vQTcpServer::vCloseServer(void)
{
    closeAllSocket();
    this->close();
    return;
}
//关闭所有，同时清空所有内存
void vQTcpServer::closeAllSocket(void)
{
    if(this->vTcpCnt>0)
    {
        this->vTcpChoose = 0;
        while(this->vTcpCnt)
        {
            this->vTcpCnt--;
            this->vTcpList[this->vTcpCnt]->abort();
            this->vTcpList[this->vTcpCnt]->close();
        }
        qDeleteAll(this->vTcpRxList);
        qDeleteAll(this->vTcpList);
        this->vTcpRxList.clear();
        this->vTcpList.clear();
        this->vTcpListCom.clear();
        emit vRxBuffChanged(nullptr);
    }
}
//新的链接接入
void vQTcpServer::vNewConnect(void)
{
    /*
     * 在这个地方，我们发现，这里是没有使用new的，这说明nextPendingConnection时，
     * QTcpSocket就已经创建了,那么对于增加一个连接，上一个连接是否还会保持
     */
    /*创建TcpSocket*/
    this->vTcpList.append(this->nextPendingConnection());
    this->vTcpRxList.append(new QByteArray);
    this->vTcpListCom.append(this->vTcpList[this->vTcpCnt]->peerPort());
    connect(this->vTcpList[this->vTcpCnt],&QTcpSocket::readyRead,[=]()
    {
        if(this->vRxHexEnable)
        {
            //二进制格式，不做处理
            this->vTcpRxList[this->vTcpChoose]->append((this->vTcpList[this->vTcpChoose]->readAll()));
        }
        else
        {
            if(!this->rxTimeStamp)
            {
                if(this->rxStampStr=="}")
                {
                    this->vTcpRxList[this->vTcpChoose]->append(
                                QString(CodeDecoding(this->vTcpList[this->vTcpChoose]->readAll()))
                                .replace(this->rxStampStr,
                                         this->rxStampStr+"\n"));
                }
                else
                {
                    this->vTcpRxList[this->vTcpChoose]->append(
                                QString(CodeDecoding(this->vTcpList[this->vTcpChoose]->readAll())));
                }
            }
            else
            {
                QString timeString;
                timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                this->vTcpRxList[this->vTcpChoose]->append(
                  QString(CodeDecoding(this->vTcpList[this->vTcpChoose]->readAll()))
                  .replace(this->rxStampStr,
                           this->rxStampStr
                           .replace("\r","")
                           .replace("\n","")
                           +timeString));
            }
        }
    });
    connect(this->vTcpList[this->vTcpCnt],&QTcpSocket::disconnected,this,&vQTcpServer::vTcpDisConnect);
    this->vTcpCnt++;
    emit vNewSocket();
}
void vQTcpServer::vSetRxStampStr(const QByteArray &str)
{
    this->rxStampStr = str;
}
void vQTcpServer::vDisEnConnect(void)
{
    for(int i=0;i<this->vTcpCnt;i++)
    {
        disconnect(this->vTcpList[i],
                   &QTcpSocket::disconnected,
                   this,&vQTcpServer::vTcpDisConnect);
    }
}
void vQTcpServer::vTcpDisConnect(void)
{
    emit vNewSocket();
}
void vQTcpServer::vConfigBuffAddr(qint16 choose)
{
    //修改显示区域地址映射
    if(this->vTcpCnt>0 && this->vTcpCnt>choose)
    {
        if(this->vTcpRxList.length()>choose)
        {
            this->vTcpChoose = choose;
            emit vRxBuffChanged(this->vTcpRxList[this->vTcpChoose]);
        }
    }
}
void vQTcpServer::vWriteStr(const QByteArray &str)
{
  if(this->vTcpList.size()>0)
  {
      this->vTcpList[this->vTcpChoose]->write(str);
  }
}
//设置相关
void vQTcpServer::vSetPort(quint16 vport_t)
{
    this->vServerPort = vport_t;
}
