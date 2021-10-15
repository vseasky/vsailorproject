#ifndef MYQCUSTOMPLOT_H
#define MYQCUSTOMPLOT_H

#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QThread>
#include <QVariant>
#include <qcustomplot.h>
#include <vtracer.h>
//#include "bsp_protocol.h"
//#include "serial_port/serial_port.h"

/*
 *****************************
 ************波形绘制************
 ******************************
 */

typedef struct
{
    Qt::Key X_Shortcut;
    Qt::Key Y_Shortcut;
    Qt::Key All_Shortcut;

}PlotShortcut;

class vQCustomPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit vQCustomPlot(QWidget *parent = nullptr);
    ~vQCustomPlot();
    QVector<double> qv_x, qv_y;
    PlotShortcut    myPlotShortcut;
    vTracer        *mxTracer = nullptr; //坐标跟随鼠标.使用时创建
    QString        *vName = nullptr;
    bool xAxisAuto = true;
    bool showEnable=true;
    uint8_t keyStatus;
    qint32 GraphShowTimerSet = 65;//波形刷新
    int lenSeasky = 0;
    QTimer  GraphShowTimer;
    bool TimerEnable;
    void TimerStart(void);
    void TimerStop(void);
    void vSetNameAddr(QString * addr);
    void setQCustomPlotTimer(qint32 tim)
    {
        GraphShowTimerSet = tim;
    }


public slots:
    void contextMenuRequest(QPoint pos);
    void moveLegend(void);
    void selectionChanged(void);
    void addRandomGraph(void);
    void removeSelectedGraph(void);
    void removeAllGraphs(void);
    void hideSelectedGraph(void);
    void hideAllGraph(void);
    void showAllGraph(void);
    bool isAllGraphHide(void);
    bool ishaveGraphHide(void);
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void colorSet(void);
    void rescaleYAxis(void);
    void GraphShowSlot(void);
    void SeaskyScope(const QVariant &rxData);
    void showCsvData(double *data,int size);
    void addSeaskyGraph(void);

    void vReadCsvData(void);
    void vSaveCsvData(void);

protected:
    //滚轮
    void wheelEvent(QWheelEvent *ev);
    //鼠标点击事件
    void mousePressEvent(QMouseEvent *ev) ;
//    //鼠标抬起事件
//    void mouseReleaseEvent(QMouseEvent *ev) ;
    //鼠标移动事件
    void mouseMoveEvent(QMouseEvent *ev) ;
//    //鼠标进入区域
//    void enterEvent(QEvent *ev);
//    //鼠标离开区域
//    void leaveEvent(QEvent *ev);
//    //键盘事件
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

signals:
    void ledShowEnable();

};

#endif // MYQCUSTOMPLOT_H
