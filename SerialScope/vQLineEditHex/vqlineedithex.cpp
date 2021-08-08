#include "vqlineedithex.h"
#include <QDebug>
vQLineEditHex::vQLineEditHex(QWidget *parent) : QLineEdit(parent)
{
    /*限制文本输入格式 "[FF ]"*/
    QRegExp regx("([A-Fa-f0-9][A-Fa-f0-9][ ]){1,}");
    validator = new QRegExpValidator(regx, this);
    connect(this,&QLineEdit::textChanged,this,&vQLineEditHex::vTextChanged);
}
void vQLineEditHex::setHexEnableAddr(bool * addr)
{
    this->hexEnable = addr;
}
void vQLineEditHex::vTextChanged(void)
{
    if(this->hexEnable==nullptr){return;}
    //非16进制格式显示处理
    if(!(*this->hexEnable))
    {
        //非16进制格式直接转换为
        *this->TextTxBuff = this->text().toUtf8();
    }
    else
    {
        *this->TextTxBuff = QByteArray::fromHex(this->text().toUtf8());
    }
}
void vQLineEditHex::setTextTxBuffAddr(QByteArray * addr)
{
    this->TextTxBuff = addr;
}
void vQLineEditHex::hexEnableChanged(void)
{
    if(this->hexEnable==nullptr){return;}
    //非文本输入切换操作
    if(!(*this->hexEnable))
    {
        this->setValidator(nullptr);
        this->setText(*this->TextTxBuff);
    }
    else
    {
        /*限制文本输入格式 "[FF ]"*/
        this->setValidator(validator);
        this->setText((*this->TextTxBuff).toHex(' ').toUpper());
    }
}
