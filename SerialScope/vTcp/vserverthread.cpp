#include "vserverthread.h"

vServerQObject::vServerQObject(QObject *parent) : QObject(parent)
{
    connect(this,&vServerQObject::vTcpTxMsg,
            &this->vServerTcp,&vQTcpServer::vWriteStr);
    connect(&this->vTxTimer,&QTimer::timeout,
            this,&vServerQObject::vTxTimeOut);
}
void vServerQObject::vTimerText(void)
{
    qDebug()<<"vTxTimertimeout";
}
//根据发送的模式选择发送的数据
void vServerQObject::vTxTimeOut(void)
{
    if(this->vTcpTxData == nullptr)
    {
        return;
    }
    switch(this->vTcpTxCfg)
    {
        case vTcpOrd:
        {
            if((*this->vTcpTxData)!="")
            {
                //16进制发送不进行编码处理
                if(this->vServerTcp.vTxHexEnable)
                {
                    emit vTcpTxMsg((*this->vTcpTxData)+LineFeed);
                }
                else
                {
                    emit vTcpTxMsg(CodeCoding((*this->vTcpTxData)+LineFeed));
                }
            }
        };break;
        case vTcpMul:
        {

            if(vLineEditTxCtr())
            {
                if((*this->vTcpLineTxData)!="")
                {
                    if(this->vServerTcp.vTxHexEnable)
                    {
                        emit vTcpTxMsg((*this->vTcpLineTxData)+LineFeed);
                    }
                    else
                    {

                        emit vTcpTxMsg(CodeCoding((*this->vTcpLineTxData)+LineFeed));
                    }
                }
            }
        };break;
        case vTcpAgr:
        {
            //按协议发送此处已在别处更改槽函数，此处不再配置，但是保留框架
        };break;
        default:break;
    }
    return;
}
//周期性调用,切换需要发送的数据,多次发送
bool vServerQObject::vLineEditTxCtr(void)
{
    static bool lastFindPos = 0;
    if(this->TcpMaxCnt>0)
    {
        QMap<qint32,bool*>::iterator findTx;
        findTx = vTcpLineMap->find(this->timerCnt+1);
        if(findTx==vTcpLineMap->end())
        {
            this->timerCnt = 0;
            return false;
        }
        while(!(*findTx.value()))//直到找到一个可以发送的
        {
            this->timerCnt++;
            if(this->timerCnt>=(this->TcpMaxCnt))
            {
                //如果第一次查询直到最后也没有查询到，那么在此需要查询2次，定位到环形列表
                if(lastFindPos==0)
                {
                    lastFindPos = 1;
                    this->timerCnt = 0;
                    return(vLineEditTxCtr());
                }
                else
                {
                    //重新从0开始查询，仍然无值，说明整个列表无值，返回false
                    lastFindPos = 0;
                    this->timerCnt = 0;
                    return false;
                }
            }
            findTx = vTcpLineMap->find(this->timerCnt+1);
            if(findTx==vTcpLineMap->end())
            {
                this->timerCnt = 0;
                return false;
            }
        }
        this->vTcpLineTxData = &vTcpLineData[this->timerCnt];
        this->timerCnt++;
        if(this->timerCnt>=(this->TcpMaxCnt))
        {
            this->timerCnt = 0;
            return false;
        }
        //查询到了，清除标志
        lastFindPos = 0;
        return true;
    }
    return false;
}

//发送多条中的一条
void vServerQObject::vLineEditTxOne(qint32 num)
{
    //超出地址，则直接返回
    if(num>=this->TcpMaxCnt)
    {
        return;
    }
    this->vTcpLineTxData = &vTcpLineData[num];
    if((*this->vTcpLineTxData)!="")
    {
        //16进制发送不进行编码处理
        if(this->vServerTcp.vTxHexEnable)
        {
            emit vTcpTxMsg((*this->vTcpLineTxData)+LineFeed);
        }
        else
        {

            emit vTcpTxMsg(CodeCoding((*this->vTcpLineTxData)+LineFeed));
        }
    }
}


void vServerQObject::vTxAddrSet(QByteArray * dataAddr)
{
    this->vTcpTxData = dataAddr;
}
void vServerQObject::vAddrSet(qint32 Cnt,
                              QByteArray * dataAddr,
                              QMap<qint32,bool*> *mapAddr)
{
    this->TcpMaxCnt    = Cnt;
    this->vTcpLineData = dataAddr;
    this->vTcpLineMap  = mapAddr;
}

void vServerQObject::vTxTimerCntSet(qint32 index_t)
{
    this->vTxTimerCnt = index_t;
}
void vServerQObject::vTxTimerStart(void)
{
    if(!this->vTxTimer.isActive())
    {
        if(this->vTxTimerCnt>0)
        {
            this->vTxTimer.start(this->vTxTimerCnt);
        }
    }
}
void vServerQObject::vTxTimerStop(void)
{
    if(this->vTxTimer.isActive())
    {
        this->vTxTimer.stop();
    }
}
vServerThread::vServerThread(QObject *parent) : QThread(parent)
{

}

