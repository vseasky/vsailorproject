#ifndef VCUSTOMWIDGET_H
#define VCUSTOMWIDGET_H

#include <QWidget>



class vCustomWidget : public  QWidget
{
    Q_OBJECT
public:
    explicit vCustomWidget(QWidget *parent = nullptr);
    uint16_t flag_t;
    void vSetButtonEnable(bool vEnable);
public:
signals:
    void vSetEnabled(bool vEnable);
    void vSetFlag(uint16_t flag);
    void vFlagChanged(void);
protected:
void paintEvent(QPaintEvent *event);
signals:

};

#endif // VCUSTOMWIDGET_H
