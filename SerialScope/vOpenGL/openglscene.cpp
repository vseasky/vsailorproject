/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "openglscene.h"
#include "model.h"

#include <QtGui>
#include <QtOpenGL>
#include <QtConcurrent>
#include <GL/glu.h>
#include <QDebug>
#include <QGraphicsObject>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

QDialog *OpenGLScene::createDialog(const QString &windowTitle) const
{
    QDialog *dialog =
            new QDialog(0, Qt::CustomizeWindowHint |
                        Qt::WindowTitleHint);

    dialog->setWindowOpacity(0.8);
    dialog->setWindowTitle(windowTitle);
    dialog->setLayout(new QVBoxLayout);

    return dialog;
}

OpenGLScene::OpenGLScene()
    : m_wireframeEnabled(false)
    , m_normalsEnabled(false)
    , m_modelColor(153, 255, 0)
    , m_backgroundColor(0, 170, 255)
    , m_model(0)
    , m_distance(1.5f)
{
#ifndef QT_NO_CONCURRENT
    connect(&m_modelLoader, SIGNAL(finished()), this, SLOT(modelLoaded()));
#endif
    QWidget *statistics = createDialog(tr("Model info"));
    statistics->layout()->setMargin(6);
    for (int i = 0; i < 3; ++i)
    {
        m_labels[i] = new QLabel;
        statistics->layout()->addWidget(m_labels[i]);
    }
    addWidget(statistics);
    QPointF pos(10, 10);
    foreach (QGraphicsItem *item, items()) {
        item->setFlag(QGraphicsItem::ItemIsMovable);
        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        const QRectF rect = item->boundingRect();
        item->setPos(pos.x() - rect.x(), pos.y() - rect.y());
        pos += QPointF(0, 10 + rect.height());
    }

    QRadialGradient gradient(40, 40, 40, 40, 40);
    gradient.setColorAt(0.2,Qt::yellow);
    gradient.setColorAt(1,Qt::transparent);

    axisCfg.TurnCfg = AxisTurnY;

    axisCfg.RotationCfg.y = 180;

    axisCfg.AxisDirX*=axisCfg.AxisCfg[axisCfg.TurnCfg][0];
    axisCfg.AxisDirY*=axisCfg.AxisCfg[axisCfg.TurnCfg][1];
    axisCfg.AxisDirZ*=axisCfg.AxisCfg[axisCfg.TurnCfg][2];
//    QString path = qApp->applicationDirPath();
    loadModel(QLatin1String(":/image/vModel/plane.obj"));
}
OpenGLScene::~OpenGLScene()
{
    delete m_labels[0];
    delete m_labels[1];
    delete m_labels[2];
    delete m_model;
}
void OpenGLScene::ZeroAngleCfg(qint8 id,qint8 cfg)
{
    switch (id)
    {
        case 0:{if(cfg==0){axisCfg.RotationCfg.x=0;}
                else {axisCfg.RotationCfg.x=180;}};break;
        case 1:{if(cfg==0){axisCfg.RotationCfg.y=0;}
                else {axisCfg.RotationCfg.y=180;}};break;
        case 2:{if(cfg==0){axisCfg.RotationCfg.z=0;}
                else {axisCfg.RotationCfg.z=180;}};break;
    }
}
void OpenGLScene::CoordinateRotation(qint8 vAxisTurn)
{
    axisCfg.TurnCfg = AxisTurn(vAxisTurn);
    axisCfg.AxisDirX*=axisCfg.AxisCfg[axisCfg.TurnCfg][0];
    axisCfg.AxisDirY*=axisCfg.AxisCfg[axisCfg.TurnCfg][1];
    axisCfg.AxisDirZ*=axisCfg.AxisCfg[axisCfg.TurnCfg][2];
}
void OpenGLScene::setCapeEuler(float yaw,float pitch,float roll)
{
    /*
     *  Yaw   - Y
     *  Pitch - X
     *  Roll  - Z
     */
    axisCfg.m_rotation.y = -yaw;
    axisCfg.m_rotation.x = pitch;
    axisCfg.m_rotation.z = roll;
}
void OpenGLScene::drawBackground(QPainter *painter, const QRectF &)
{
    glClearColor(m_backgroundColor.redF(),m_backgroundColor.greenF(),m_backgroundColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if (!m_model)
        return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(70,width()/height(),0.01, 1000);


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(m_modelColor.redF(),m_modelColor.greenF(),m_modelColor.blueF(), 1.0f);
    glTranslatef(0, 0,-m_distance);
    glRotatef((axisCfg.m_rotation.x-axisCfg.RotationCfg.x)*axisCfg.AxisCfg[axisCfg.TurnCfg][0],1,0,0);
    glRotatef((-axisCfg.m_rotation.y-axisCfg.RotationCfg.y)*axisCfg.AxisCfg[axisCfg.TurnCfg][1],0,1,0);
    glRotatef((-axisCfg.m_rotation.z-axisCfg.RotationCfg.z)*axisCfg.AxisCfg[axisCfg.TurnCfg][2],0,0,1);

    glEnable(GL_MULTISAMPLE);
    m_model->render(m_wireframeEnabled,m_normalsEnabled);
    glDisable(GL_MULTISAMPLE);

    /*画三条线*/
    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(-this->axisCfg.Size,0,0);
    glVertex3f(this->axisCfg.Size,0,0);

    glColor3f(0,1,0);
    glVertex3f(0,-this->axisCfg.Size,0);
    glVertex3f(0,this->axisCfg.Size,0);

    glColor3f(0,0,1);
    glVertex3f(0,0,-this->axisCfg.Size);
    glVertex3f(0,0,this->axisCfg.Size);
    glEnd();
    int  i =  0 ;
    float length1 = this->axisCfg.Size;
    float length2 = this->axisCfg.ConeCfg*this->axisCfg.Size;
    glBegin(GL_QUAD_STRIP);  //连续填充四边形串
    for  (i = 0;i <= 360 ;i +=  2)
    {
        float  p = i *  3.14  /  180 ;
        glColor3f(1,0,0);
        glVertex3f(this->axisCfg.AxisDirX*length1,0,0);
        glVertex3f(this->axisCfg.AxisDirX*length2,0.025*sin(p),0.025*cos(p));
    }
    glEnd();
    glBegin(GL_QUAD_STRIP); //连续填充四边形串
    for  (i = 0;i <= 360 ;i +=  2)
    {
        float  p = i *  3.14  /  180 ;
        glColor3f(0,1,0);
        glVertex3f(0,this->axisCfg.AxisDirY*length1,0);
        glVertex3f(0.025*sin(p),this->axisCfg.AxisDirY*length2,0.025*cos(p));
    }
    glEnd();
    glBegin(GL_QUAD_STRIP); //连续填充四边形串
    for  (i = 0;i <= 360 ;i +=  2)
    {
        float  p = i *  3.14  /  180 ;
        glColor3f(0,0,1.0f);
        glVertex3f(0,0,this->axisCfg.AxisDirZ*length1);
        glVertex3f(0.025*sin(p),0.025*cos(p),this->axisCfg.AxisDirZ*length2);
    }
    glEnd();

    /*
     *  Yaw   - Y
     *  Pitch - X
     *  Roll  - Z
     */
    m_labels[0]->setText(QString::fromLocal8Bit("Yaw  :%1 °").arg(axisCfg.m_rotation.y));
    m_labels[1]->setText(QString::fromLocal8Bit("Pitch:%1 °").arg(axisCfg.m_rotation.x));
    m_labels[2]->setText(QString::fromLocal8Bit("Roll :%1 °").arg(axisCfg.m_rotation.z));

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    QTimer::singleShot(20,this,SLOT(update()));
}

void OpenGLScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);
}

static Model *loadModel(const QString &filePath)
{
    return new Model(filePath);
}
void OpenGLScene::vloadModel(void)
{
    loadModel(QFileDialog::getOpenFileName(0,QString::fromLocal8Bit("选择模型"), QString(), QLatin1String("*.obj")));
}
void OpenGLScene::loadModel()
{
    loadModel(QFileDialog::getOpenFileName(0,QString::fromLocal8Bit("选择模型"), QString(), QLatin1String("*.obj")));
}

void OpenGLScene::loadModel(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

//    m_modelButton->setEnabled(false);
    QApplication::setOverrideCursor(Qt::BusyCursor);
#ifndef QT_NO_CONCURRENT
    m_modelLoader.setFuture(QtConcurrent::run(::loadModel,filePath));
#else
    setModel(::loadModel(filePath));
    modelLoaded();
#endif
}

void OpenGLScene::modelLoaded()
{
#ifndef QT_NO_CONCURRENT
    setModel(m_modelLoader.result());
#endif
//    m_modelButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}

void OpenGLScene::setModel(Model *model)
{
    delete m_model;
    m_model = model;
    update();
}

//不使用
void OpenGLScene::enableWireframe(bool enabled)
{
    m_wireframeEnabled = enabled;
    update();
}
//不使用
void OpenGLScene::enableNormals(bool enabled)
{
    m_normalsEnabled = enabled;
    update();
}

//设置模型颜色
void OpenGLScene::setModelColor()
{
    const QColor color = QColorDialog::getColor(m_modelColor);
    if (color.isValid())
    {
        m_modelColor = color;
        update();
    }
}
//设置背景颜色
void OpenGLScene::setBackgroundColor()
{
    const QColor color = QColorDialog::getColor(m_backgroundColor);
    if (color.isValid()) {
        m_backgroundColor = color;
        update();
    }
}
//不知道这里事件为什么不继续传递，因此只能多加一个信号与槽
void OpenGLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
    if (event->isAccepted())
        return;
    float x,y;
    x = (event->scenePos().x()-this->width()/2)/(this->width());
    y = (event->scenePos().y()-this->height()/2)/(this->height());
    if (event->buttons() & Qt::LeftButton)
    {
        if(mouseEventCfg==Config1)
        {
            const QPointF delta = event->scenePos() - event->lastScenePos();
            const Point3d angularImpulse = Point3d(delta.y(), delta.x(), 0) * 0.1;
            axisCfg.m_rotation -= angularImpulse;
            update();
        }
        else if(mouseEventCfg==Config2)
        {
            QPoint mouseMoveAddr = QPoint(event->scenePos().x(),event->scenePos().y());
            emit mouseMove(mouseMoveAddr);
        }
    }
    else
    {
        if((abs(x)<0.4)&&(abs(y)<0.4))
        {
            mouseEventCfg=Config1;
        }
        else
        {
            mouseEventCfg=Config2;
        }
    }
    event->accept();
}
void OpenGLScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsScene::wheelEvent(event);
    if (event->isAccepted())
        return;
    m_distance *= qPow(1.2, -event->delta() / 120);
    event->accept();
    update();
}
