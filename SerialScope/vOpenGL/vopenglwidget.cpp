#include "vopenglwidget.h"
#include <QDebug>
//提供给欧拉角显示的无边框窗口
vOpenGlWidget::vOpenGlWidget(QWidget *parent)
    :QGraphicsView(parent)
{
    //会有一点白边,可以设置完全无边框的
    //但是相比之下设置拉伸和移动功能还要添加较多东西
    this->setWindowFlags(Qt::CustomizeWindowHint);
    this->setMinimumSize(45,45);//设置最小尺寸
    setMouseTracking(true);
}

vOpenGlWidget::~vOpenGlWidget()
{

}

void vOpenGlWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        clickPos=event->pos();
    }
    if(event->button()==Qt::RightButton)
    {
        QMenu *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);

        QMenu *menuchoose = menu->addMenu(QString::fromLocal8Bit("设置数据"));
        QMenu *dataYaw   = menuchoose->addMenu(QString::fromLocal8Bit("Yaw  :")+QString("Rx: %1").arg(this->checked[0]));
        QMenu *dataPitch = menuchoose->addMenu(QString::fromLocal8Bit("Pitch:")+QString("Rx: %1").arg(this->checked[1]));
        QMenu *dataRoll  = menuchoose->addMenu(QString::fromLocal8Bit("Roll :")+QString("Rx: %1").arg(this->checked[2]));
        QAction * thisAction;
        for(qint8 i=0;i<24;i++)
        {
            thisAction = dataYaw->addAction(QString("Rx: %1").arg(i+1),this,[=]()
            {
                this->checked[0]=i+1;
            });
            thisAction->setCheckable(true);
            thisAction->setChecked(this->checked[0]==i+1);
        }
        for(qint8 i=0;i<24;i++)
        {
            thisAction = dataPitch->addAction(QString("Rx: %1").arg(i+1),this,[=]()
            {
                this->checked[1]=i+1;
            });
            thisAction->setCheckable(true);
            thisAction->setChecked(this->checked[1]==i+1);
        }

        for(qint8 i=0;i<24;i++)
        {
            thisAction = dataRoll->addAction(QString("Rx: %1").arg(i+1),this,[=]()
            {
                this->checked[2]=i+1;
            });
            thisAction->setCheckable(true);
            thisAction->setChecked(this->checked[2]==i+1);
        }
        QMenu *Coordinate = menu->addMenu(QString::fromLocal8Bit("坐标轴旋转"));
        Coordinate->addAction(QString::fromLocal8Bit("绕X轴转180°"),this,[=]()
        {
           emit vAxisTurnChange(2);
        });
        Coordinate->addAction(QString::fromLocal8Bit("绕Y轴转180°"),this,[=]()
        {
           emit vAxisTurnChange(1);
        });
        Coordinate->addAction(QString::fromLocal8Bit("绕Z轴转180°"),this,[=]()
        {
           emit vAxisTurnChange(3);
        });
        QMenu *zeroX = menu->addMenu(QString::fromLocal8Bit("X轴零角度"));
        QMenu *zeroY = menu->addMenu(QString::fromLocal8Bit("Y轴零角度"));
        QMenu *zeroZ = menu->addMenu(QString::fromLocal8Bit("Z轴零角度"));
        zeroX->addAction(QString::fromLocal8Bit("0°"),this,[=]()
        {
            emit ZeroAngleCfg(0,0);
        });
        zeroX->addAction(QString::fromLocal8Bit("180°"),this,[=]()
        {
            emit ZeroAngleCfg(0,1);
        });
        zeroY->addAction(QString::fromLocal8Bit("0°"),this,[=]()
        {
            emit ZeroAngleCfg(1,0);
        });
        zeroY->addAction(QString::fromLocal8Bit("180°"),this,[=]()
        {
            emit ZeroAngleCfg(1,1);
        });
        zeroZ->addAction(QString::fromLocal8Bit("0°"),this,[=]()
        {
            emit ZeroAngleCfg(2,0);
        });
        zeroZ->addAction(QString::fromLocal8Bit("180°"),this,[=]()
        {
            emit ZeroAngleCfg(2,1);
        });
        menu->addAction(QString::fromLocal8Bit("导入模型"), this, [=]()
        {
            emit loadMode();
        });

        menu->addAction(QString::fromLocal8Bit("模型颜色"), this, [=]()
        {
            emit setModelColor();
        });
        menu->addAction(QString::fromLocal8Bit("背景颜色"), this, [=]()
        {
            emit setBackgroundColor();
        });
        menu->addAction(QString::fromLocal8Bit("关闭窗口"), this, [=]()
        {
            this->hide();
//            this->close();
        });
        menu->popup(this->mapToGlobal(event->pos()));
    }
    event->accept();
}
void vOpenGlWidget::getCapeEuler(const QVariant &rxData)
{
    QVector<float> rxSeasky = rxData.value< QVector<float> >();
    if((rxSeasky.size()>0)&&(rxSeasky.size()<=24))
    {
        if(((this->checked[0]!=0)&&(this->checked[0]<=rxSeasky.size()))&&
           ((this->checked[1]!=0)&&this->checked[1]<=rxSeasky.size())&&
           ((this->checked[2]!=0)&&this->checked[2]<=rxSeasky.size()))
        {
            yaw   = rxSeasky[this->checked[0]-1];
            pitch = rxSeasky[this->checked[1]-1];
            roll  = rxSeasky[this->checked[2]-1];
            emit setCapeEuler(yaw,pitch,roll);
        }
    }
}
void vOpenGlWidget::vmouseMoveEvent(QPoint event)
{
    float x,y;
    x = float(event.x()-this->width()/2)/(this->width());
    y = float(event.y()-this->height()/2)/(this->height());
    move(event+this->pos()-clickPos);
}

