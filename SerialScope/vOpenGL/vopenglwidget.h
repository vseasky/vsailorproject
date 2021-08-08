#ifndef VOPENGLWIDGET_H
#define VOPENGLWIDGET_H


#include <QMouseEvent>
#include <QGraphicsView>
#include <QMenu>

class vOpenGlWidget : public QGraphicsView
{
    Q_OBJECT
public:
    vOpenGlWidget(QWidget *parent = nullptr);
    ~vOpenGlWidget();
    QPoint clickPos;
    float yaw,pitch,roll;
    qint8  checked[3]={0};
    void vmouseMoveEvent(QPoint event);
public slots:
    void getCapeEuler(const QVariant &rxData);
protected:
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event)
    {
        if (scene())
            scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
        QGraphicsView::resizeEvent(event);
    }
signals:
    void setCapeEuler(float yaw,float pitch,float roll);
    void loadMode(void);
    void setModelColor(void);
    void setBackgroundColor(void);
    void vAxisTurnChange(qint8 changed);
    void ZeroAngleCfg(qint8 id,qint8 cfg);
private:

};
#endif // WIDGET_H
