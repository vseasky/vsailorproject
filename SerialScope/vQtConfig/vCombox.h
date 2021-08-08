#ifndef V_COMBOX_H
#define V_COMBOX_H

#include<QComboBox>

class vCombox : public QComboBox
{
    Q_OBJECT
public:
    explicit vCombox(QWidget *parent = nullptr);
    void     setItem();
protected:
//    void mousePressEvent(QMouseEvent *ev);

//private slots:
//    void Activated(int index);
signals:


};
#endif // MY_COMBOX_H
