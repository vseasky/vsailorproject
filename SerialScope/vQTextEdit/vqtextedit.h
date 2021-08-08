#ifndef VQTEXTEDIT_H
#define VQTEXTEDIT_H

#include <QTextEdit>

class vQTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit vQTextEdit(QWidget *parent = nullptr);
    QByteArray  TextTxBuff;
    bool * hexEnable;//连接到外部的hex发送控制，指针赋值，只需修改总控
    void setHexEnableAddr(bool * addr);
    void hexTextLimit(const QString & hexStr);
public slots:
    void vTextChanged(void);
    void hexEnableChanged(void);
signals:

};

#endif // VQTEXTEDIT_H
