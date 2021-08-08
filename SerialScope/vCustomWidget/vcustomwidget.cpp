#include <vcustomwidget.h>
#include <QApplication>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QBitmap>

//使用QPushButton将16进制按位显示
vCustomWidget::vCustomWidget(QWidget *parent) : QWidget(parent)
{
    QPixmap pix;
    QHBoxLayout * vQHBoxLayout = new QHBoxLayout(this);
    QPushButton * vQPushbutton;
    this->setAutoFillBackground(true);
    for(qint16 i=0;i<16;i++)
    {
        vQPushbutton = new QPushButton(this);
        vQPushbutton->setMinimumSize(18,18);
        vQPushbutton->setMaximumSize(18,18);
        vQPushbutton->setCheckable(true);
        vQHBoxLayout->setMargin(-3);
        vQHBoxLayout->setSpacing(3);
        vQHBoxLayout->addWidget(vQPushbutton);
        connect(this,&vCustomWidget::vSetEnabled,[=](bool vEnable)
        {
            vQPushbutton->setEnabled(vEnable);
        });
        connect(vQPushbutton,&QPushButton::released,[=]()
        {
            this->flag_t = (this->flag_t&(~(uint16_t(0X0001)<<i)))|
                           (vQPushbutton->isChecked()<<i);
            emit  vFlagChanged();
        });
        connect(this,&vCustomWidget::vSetFlag,[=](uint16_t flag)
        {
            this->flag_t = flag;
            vQPushbutton->setChecked(bool((this->flag_t>>i)&0X0001));
        });
        vQPushbutton->setStyleSheet("");
    }
    this->setLayout(vQHBoxLayout);
}
void vCustomWidget::vSetButtonEnable(bool vEnable)
{
    emit vSetEnabled(vEnable);
}
void vCustomWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
}
