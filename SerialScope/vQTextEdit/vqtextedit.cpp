#include "vqtextedit.h"
#include <QDebug>
bool checkHexChar(char ch)
{
    if ((ch >= '0') && (ch <= '9')) {
        return true;
    } else if ((ch >= 'A') && (ch <= 'F')) {
        return true;
    } else if ((ch >= 'a') && (ch <= 'f')) {
        return true;
    } else {
        return false;
    }
}
vQTextEdit::vQTextEdit(QWidget *parent) : QTextEdit(parent)
{
    connect(this,&QTextEdit::textChanged,this,&vQTextEdit::vTextChanged);
}
//使用前必须绑定此地址
void vQTextEdit::setHexEnableAddr(bool * addr)
{
    this->hexEnable = addr;
}
//如果文本发生了改变，更新缓冲TexeTxBuff
void vQTextEdit::vTextChanged(void)
{
    //文本输入->读取当前文本，更新需要显示的文本，以及限制文本输入
    if(this->hexEnable==nullptr){return;}
    //非16进制格式显示处理
    if(!(*this->hexEnable))
    {
        //非16进制格式直接转换为
        this->TextTxBuff = this->toPlainText().toUtf8();
    }
    else
    {
        this->hexTextLimit(this->toPlainText());
        this->TextTxBuff = QByteArray::fromHex(this->toPlainText().toUtf8());
    }
};
//16进制格式时限制文本输入
void vQTextEdit::hexTextLimit(const QString & hexStr)
{
    /*限制文本输入格式 "[FF ]"*/
    QTextCursor cursor = this->textCursor();
    if(hexStr!=NULL)
    {
       static int last_len=0;      //上一次最后空格的位置
       int len = hexStr.length();
       if(!(hexStr.at(len-1)==" "))//本次加入的不是空格
       {
           if(len-last_len <= 2)
           {
               if(!checkHexChar(hexStr.at(len-1).toLatin1()))
               {
                   cursor.deletePreviousChar();
               }
           }
           else
           {
                cursor.deletePreviousChar();
           }
       }
       else
       {
            last_len=len;
       }
    }
}
//显示格式切换时单次调用，需要触发
void vQTextEdit::hexEnableChanged(void)
{
   if(this->hexEnable==nullptr){return;}
    //非文本输入切换操作
    if(!(*this->hexEnable))
    {
        this->setPlainText(this->TextTxBuff);
    }
    else
    {
        this->setPlainText(this->TextTxBuff.toHex(' ').toUpper());
    }
}
