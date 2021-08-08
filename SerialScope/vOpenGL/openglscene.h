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

#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H

#include "point3d.h"

#include <QGraphicsScene>
#include <QLabel>
#include <QTime>
#include <QMenu>

#ifndef QT_NO_CONCURRENT
#include <QFutureWatcher>
#endif

class Model;
typedef enum
{
    AxisNull = 0,
    AxisTurnY = 1,
    AxisTurnX = 2,
    AxisTurnZ = 3,
}AxisTurn;
typedef struct
{
    float Size = 0.65f;
    float ConeCfg = 0.8f;
    const int AxisCfg[4][3]={
                             {1,1,1},  //正常
                             {-1,1,-1},//绕Y轴旋转180°
                             {1,-1,-1},//绕X轴旋转180°
                             {-1,-1,1} //绕Z轴旋转180°
                             };
    AxisTurn TurnCfg;
    float AxisDirX=1;
    float AxisDirY=1;
    float AxisDirZ=1;
    Point3d m_rotation;
    Point3d RotationCfg;
}vOpenGlConfig;
typedef enum
{
    Null=0,
    Config1,
    Config2
}vMouseEvent;

class OpenGLScene : public QGraphicsScene
{
    Q_OBJECT

public:
    OpenGLScene();
    ~OpenGLScene();
    QPoint clickPos;
    vMouseEvent mouseEventCfg = Null;
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawForeground(QPainter *painter, const QRectF &rect);

public slots:
    void setCapeEuler(float yaw,float pitch,float roll);
    void ZeroAngleCfg(qint8 id,qint8 cfg);
    void CoordinateRotation(qint8 vAxisTurn);
    void enableWireframe(bool enabled);
    void enableNormals(bool enabled);
    void setModelColor();
    void setBackgroundColor();
    void loadModel();
    void vloadModel(void);
    void loadModel(const QString &filePath);
    void modelLoaded();

signals:
    void mouseMove(QPoint enent);
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

private:
    QDialog *createDialog(const QString &windowTitle) const;

    void setModel(Model *model);

    bool m_wireframeEnabled;
    bool m_normalsEnabled;

    QColor m_modelColor;
    QColor m_backgroundColor;

    Model *m_model;

    int m_lastTime;
    int m_mouseEventTime;

    float m_distance;
    vOpenGlConfig axisCfg;

    QLabel  *m_labels[3];

#ifndef QT_NO_CONCURRENT
    QFutureWatcher<Model *> m_modelLoader;
#endif
};

#endif
