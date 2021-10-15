/*
 ************************************** Copyright ******************************
 * @FileName     :  vqtcpserver.h
 * @Author       :  Seasky.Liu
 * @Date         :  2021/8/30
 * @Time         :  19:28
 * @Email        :  xxx.
 ******************************************************************************
 */

#ifndef VQTCPSERVER_H
#define VQTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include <vcodeconverter.h>
#include <vdebug.h>

/*
 *QTcpServer 实现监听端口，监听到端口会触发 newConnection 信号
 *  当触发newConnection 信号之后、
 *  你可以使用this->nextPendingConnection()获取新增的QTcpSocket
 */
class vQTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit vQTcpServer(QTcpServer *parent = nullptr);
    ~vQTcpServer();
    /*QList<*>需要先qDeleteAll后clear*/
    QList<QTcpSocket*>  vTcpList;   //用于操作多个客户端
    QList<QByteArray*>  vTcpRxList;
    QVector<quint16>    vTcpListCom;//记录端口号
    quint16             vTcpChoose = 0;
    quint16             vTcpCnt = 0;    //记录Socket个数

    bool                vRxHexEnable = false;
    bool                vTxHexEnable = false;
    bool                rxTimeStamp  = false;
    QByteArray          rxStampStr = "\r\n";
    vSerialCodeModeEnum vTcpCodeMode = SerialCodeNo;
    bool vOpenServer(void);
    void vCloseServer(void);
    void vConfigBuffAddr(qint16 choose);
    void vSetPort(quint16 vport_t);
    void vWriteStr(const QByteArray &str);
    void closeAllSocket(void);
    void vDisEnConnect(void);
    void vSetRxStampStr(const QByteArray &str);
public slots:
    void vTcpDisConnect(void);
    void vNewConnect(void);
private:
    quint16 vServerPort;
signals:
    void vNewSocket(void);
    void vRxBuffChanged(QByteArray * arr);
    void vMoveThread(QTcpSocket *vSocket);
};

#endif // VQTCPSERVER_H
