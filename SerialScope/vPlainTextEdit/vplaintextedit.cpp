#include "vplaintextedit.h"
#include <QDebug>
//自定义的可以刷新大量数据的QPlainTextEdit显示窗口，定时器开启自动刷新
static int32_t MAXRANGESIZE = 8192;
vPlainTextEdit::vPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    this->TimerCfg = 100;//默认100，后续外层通过setTimerCfg设定时间，需要重启
    this->TimerCtr.setTimerType(Qt::PreciseTimer);
    this->TimerStop();
    connect(&this->TimerCtr,&QTimer::timeout,
            this,&vPlainTextEdit::vTimerOut);
    connect(this->verticalScrollBar(),&QAbstractSlider::actionTriggered,
            this,&vPlainTextEdit::autoScroll);
    this->setReadOnly(true);
//    this->TimerStart();
}
void vPlainTextEdit::setHexEnableAddr(bool * addr)
{
    this->hexEnable = addr;
}

void vPlainTextEdit::SetShowBuffAddr(QByteArray *Buff)
{
    this->vShowBuff = Buff;
}
void vPlainTextEdit::setTimerCfg(qint32 outtime)
{
    this->TimerCfg = outtime;
}
void vPlainTextEdit::TimerStart(void)
{
    this->TimerEnable = true;
    if(!this->TimerCtr.isActive())
    {
        this->TimerCtr.start(this->TimerCfg);
    }
}
void vPlainTextEdit::TimerStop(void)
{
    this->TimerEnable = false;
    if(this->TimerCtr.isActive())
    {
        this->TimerCtr.stop();
    }
}
//定时器的开启仅用于追加时的实时刷新
void vPlainTextEdit::vTimerOut(void)
{
    qint64 showBuffMaxPos;
    showBuffMaxPos = this->vShowBuff->size();
    if(this->showPosMax>showBuffMaxPos)
    {
        this->showPosMax = showBuffMaxPos;
        this->showPosMin = 0;
    }
    if((this->showPosMax<=showBuffMaxPos)&&(this->TimerEnable))
    {
        this->showPosMax = showBuffMaxPos;
        if((this->showPosMax-this->showPosMin)>MAXRANGESIZE)
        {
            //不能够在容量之内显示
            this->showPosMin = this->showPosMax-MAXRANGESIZE;
        }
        this->vUpdataShow();//更新显示
        this->moveCursor(QTextCursor::End);
    }
}
void vPlainTextEdit::autoScroll(int action)
{
   if(bar->maximum() == 0)
        return;
   if(action == QAbstractSlider::SliderSingleStepAdd ||
      action == QAbstractSlider::SliderSingleStepSub||
      action == QAbstractSlider::SliderPageStepAdd||
      action == QAbstractSlider::SliderPageStepSub||
      action == QAbstractSlider::SliderMove)
   {
       qint32 value  = bar->value();
       qint32 barMax = bar->maximum();
       if((!this->TimerEnable)&&(value<barMax*0.1)&&this->showPosMin)
       {
           if(this->showPosMax-this->showPosMin<this->showPosMax/2)
           {
                this->showPosMin = this->showPosMin*0.5;
           }
           else
           {
               this->showPosMin = 0;
           }
           this->vUpdataShow();
           bar->setValue(bar->maximum()-barMax);
       }
       //是否开启定时器刷新
       if((this->TimerEnable==true)&&(value!=barMax))
       {
           TimerStop();
       }
       else if((this->TimerEnable==false)
               &&((value==barMax))
               &&(this->vShowBuff->size()-this->showPosMax>0))
       {
           TimerStart();
       }
   }
}
void vPlainTextEdit::vUpdataShow(void)
{
    if(this->hexEnable==nullptr)return;
    if((this->showPosMax<=this->vShowBuff->size())&&(this->showPosMin<=this->showPosMax))
    {
        if(!(*this->hexEnable))
        {
            this->setPlainText(this->vShowBuff->mid(this->showPosMin,this->showPosMax));
        }
        else
        {
            this->setPlainText(this->vShowBuff->mid(this->showPosMin,this->showPosMax).toHex(' ').toUpper());
        }
    }
}
//清除数据之后无论如何都要开启定时器，否则将无法自动触发定时器，
//因为定时器的启动和暂停加入了由滑块检测自动控制,而此时滑块无效
void vPlainTextEdit::clearBuff(void)
{
    this->showPosMin = 0;
    this->showPosMax = 0;
    this->vShowBuff->clear();
    this->clear();
    TimerStart();
}
//根据hex标志刷新窗口
void vPlainTextEdit::hexEnableChanged(void)
{
    //非文本输入切换操作
    if(this->hexEnable==nullptr)return;
    if(this->showPosMin<=this->showPosMax)
    {
        this->showPosMin = 0;
        this->showPosMax = 0;
        if(!(*this->hexEnable))
        {
            this->setPlainText(this->vShowBuff->mid(this->showPosMin,this->showPosMax));
        }
        else
        {
            this->setPlainText(this->vShowBuff->mid(this->showPosMin,this->showPosMax).toHex(' ').toUpper());
        }
        TimerStart();
    }
}
