#include "vtcpctr.h"

vTcpCtr::vTcpCtr(QObject *parent) : QObject(parent)
{
    this->vServerTcpCtr.setObjectName("vTcpCtr");
    this->vServerThreadCtr.moveToThread(&this->vServerThreadCtr);
    this->vServerTcpCtr.moveToThread(&this->vServerThreadCtr);
    this->moveToThread(&this->vServerThreadCtr);
    this->vServerThreadCtr.start();
}

vTcpCtr::~vTcpCtr(void)
{
    this->vServerThreadCtr.exit();
    this->vServerThreadCtr.wait();
}
