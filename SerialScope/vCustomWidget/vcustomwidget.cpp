#include <vcustomwidget.h>
#include <QApplication>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QBitmap>
#include <QFileDialog>

//使用QPushButton将16进制按位显示
vCustomWidget::vCustomWidget(QWidget *parent) : QWidget(parent)
{
//    /*多LineEdit配置*/
    this->MultPleMaxCnt = 100;
    QVBoxLayout     * vQVBoxLayout = new QVBoxLayout(this);
    QLineEdit       * vQlineEdit;
    QLineEdit       * vQLabel;
    QLineEdit       *vEquipmentTypeEdit;  //设备类型
    QLineEdit       *vEquipmentIdEdit; 	  //设备ID
    QLineEdit       *vDataIdEdit;         //数据ID
    QLineEdit       *vDataLenEdit;        //数据长度
    QLineEdit       *vEquipmentTypeLabel; //设备类型
    QLineEdit       *vEquipmentIdLabel;   //设备ID
    QLineEdit       *vDataIdLabel;        //数据ID
    QLineEdit       *vDataLenLabel;       //数据长度
    QCheckBox       *vQCheckBox;
    QPushButton     *vQPushButton;
    QWidget         * vQWidgetCtr;
    QHBoxLayout     * vQHBoxLayoutCtr;
    QWidget         * vQWidget;
    QHBoxLayout     * vQHBoxLayout;

    vQVBoxLayout->setAlignment(Qt::AlignTop);
    vQVBoxLayout->setContentsMargins(0,0,0,0);
    vQVBoxLayout->setMargin(0);
    vQVBoxLayout->setSpacing(0);

    for(uint8_t Qv_I = 0;Qv_I<100;Qv_I++)
    {
        vQWidget     = new QWidget(this);
        vQHBoxLayout = new QHBoxLayout(this);
        vQWidgetCtr  = new QWidget(this);
        vQHBoxLayoutCtr = new QHBoxLayout(this);
        //设置样式
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vQHBoxLayout->setMargin(0);
        vQHBoxLayout->setSpacing(0);
        vQHBoxLayout->setAlignment(Qt::AlignLeft);

        vQCheckBox = new QCheckBox(this);
        vQPushButton = new QPushButton(this);
        vQCheckBox->setContentsMargins(0,0,0,0);

        vEquipmentTypeLabel  = new QLineEdit(this);
        vEquipmentIdLabel    = new QLineEdit(this);
        vDataIdLabel         = new QLineEdit(this);
        vDataLenLabel        = new QLineEdit(this);

        vEquipmentTypeLabel->setMinimumSize(84,32);
        vEquipmentTypeLabel->setReadOnly(true);
        vEquipmentTypeLabel->setAlignment(Qt::AlignLeft);
        vEquipmentTypeLabel->setText("Equ_Type");

        vEquipmentIdLabel->setMinimumSize(72,32);
        vEquipmentIdLabel->setReadOnly(true);
        vEquipmentIdLabel->setAlignment(Qt::AlignLeft);
        vEquipmentIdLabel->setText("Equ_Id");

        vDataIdLabel->setMinimumSize(72,32);
        vDataIdLabel->setReadOnly(true);
        vDataIdLabel->setAlignment(Qt::AlignLeft);
        vDataIdLabel->setText("DataId");

        vDataLenLabel->setMinimumSize(72,32);
        vDataLenLabel->setReadOnly(true);
        vDataLenLabel->setAlignment(Qt::AlignLeft);
        vDataLenLabel->setText("DataLen");


        vEquipmentTypeEdit  = new QLineEdit(this);
        vEquipmentIdEdit    = new QLineEdit(this);
        vDataIdEdit         = new QLineEdit(this);
        vDataLenEdit        = new QLineEdit(this);

        vEquipmentTypeEdit->setMinimumSize(128,32);
        vEquipmentTypeEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");
        vEquipmentIdEdit->setMinimumSize(128,32);
        vEquipmentIdEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");
        vDataIdEdit->setMinimumSize(128,32);
        vDataIdEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");
        vDataLenEdit->setMinimumSize(128,32);
        vDataLenEdit->setStyleSheet("background-color:#FFFFFF;"
                                  "color:#FF4500;");
        //添加元素
        vQHBoxLayoutCtr->addWidget(vQCheckBox);
        vQHBoxLayoutCtr->addWidget(vQPushButton);
        vQWidgetCtr->setLayout(vQHBoxLayoutCtr);
        vQHBoxLayout->addWidget(vQWidgetCtr);
        vQHBoxLayout->addWidget(vEquipmentTypeLabel);
        vQHBoxLayout->addWidget(vEquipmentTypeEdit);
        vQHBoxLayout->addWidget(vEquipmentIdLabel);
        vQHBoxLayout->addWidget(vEquipmentIdEdit);
        vQHBoxLayout->addWidget(vDataIdLabel);
        vQHBoxLayout->addWidget(vDataIdEdit);
        vQHBoxLayout->addWidget(vDataLenLabel);
        vQHBoxLayout->addWidget(vDataLenEdit);
        for(uint8_t Qh_I = 0;Qh_I<24;Qh_I++)
        {
            vQLabel     = new QLineEdit(this);
            vQlineEdit  = new QLineEdit(this);
            vQLabel->setMinimumSize(60,32);
            vQLabel->setReadOnly(true);
            vQLabel->setAlignment(Qt::AlignCenter);
            vQLabel->setText(QString("%1").arg(Qv_I,2,10,QLatin1Char('0'))+":"+QString("%1").arg(Qh_I,2,10,QLatin1Char('0')));
            vQlineEdit->setMinimumSize(128,32);
            vQlineEdit->setStyleSheet("background-color:#FFFFFF;"
                                      "color:#FF4500;");
            vQHBoxLayout->addWidget(vQLabel);
            vQHBoxLayout->addWidget(vQlineEdit);
        }
        vQWidget->setLayout(vQHBoxLayout);
        vQWidget->setAutoFillBackground(true);
        vQVBoxLayout->addWidget(vQWidget);
    }
    this->setLayout(vQVBoxLayout);
    this->setAutoFillBackground(true);
}
void vCustomWidget::vSetButtonEnable(bool vEnable)
{
    emit vSetEnabled(vEnable);
}
void vCustomWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
}
