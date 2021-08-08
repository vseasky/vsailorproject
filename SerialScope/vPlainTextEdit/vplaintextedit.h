#ifndef VPLAINTEXTEDIT_H
#define VPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <QTimer>
#include <QScrollBar>
class vPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit vPlainTextEdit(QWidget *parent = nullptr);
    QByteArray * vShowBuff;
    QScrollBar * bar = this->verticalScrollBar();
    bool   *hexEnable = nullptr;
    QTimer TimerCtr;
    bool   TimerEnable;
    qint32 TimerCfg;
    qint64 showPosMin=0,showPosMax=0;
    void setHexEnableAddr(bool * addr);
    void SetShowBuffAddr(QByteArray *Buff);
    void setTimerCfg(qint32 outtime);
    void vUpdataShow(void);
    void TimerStart(void);
    void TimerStop(void);
public slots:
    void autoScroll(int action);
    void clearBuff(void);
    void vTimerOut(void);
    void hexEnableChanged(void);
signals:

};

#endif // VPLAINTEXTEDIT_H
