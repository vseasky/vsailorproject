#ifndef VQLINEEDITHEX_H
#define VQLINEEDITHEX_H

#include <QLineEdit>
#include <QValidator>
class vQLineEditHex : public QLineEdit
{
    Q_OBJECT
public:
    explicit vQLineEditHex(QWidget *parent = nullptr);
    bool * hexEnable;
    //选中群发
    bool   isTxEnable;//是否被选中
    qint32 QLineId;

    QByteArray * TextTxBuff;
    QValidator * validator;
    void setHexEnableAddr(bool * addr);
    void setTextTxBuffAddr(QByteArray * addr);
public slots:
    void vTextChanged(void);
    void hexEnableChanged(void);
signals:

};

#endif // VQLINEEDITHEX_H
