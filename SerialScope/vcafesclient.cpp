#include "vcafesclient.h"
#include <QHostInfo>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
vCafesClient::vCafesClient(QObject *parent) : QObject(parent)
{

}
/*
 * 名称:get_localmachine_name
 * 功能:获取本机机器名称
 * 参数:no
 * 返回:QString
 */
QString vCafesClient::get_localmachine_name()
{
    QString machineName     = QHostInfo::localHostName();
    return machineName;
}
/*
 * 名称:get_localmachine_ip
 * 功能:获取本机的IP地址
 * 参数:no
 * 返回:QString
 */
QString vCafesClient::get_localmachine_ip()
{
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    return ipAddress;
}
/*
 * 名称:get_localmachine_mac
 * 功能:获取本机的MAC地址
 * 参数:no
 * 返回:void
 */
QString vCafesClient::get_localmachine_mac()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
         // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
         if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
         {
             strMacAddr = nets[i].hardwareAddress();
             break;
         }
    }
    return strMacAddr;
}

