#ifndef MYTRACER_H
#define MYTRACER_H

#include <QObject>
#include <qcustomplot.h>

/*
 **********************************************
 ************myQCustomPlot的移动光标************
 **********************************************
 */

enum TracerType
{
    XAxisTracer,
    YAxisTracer,
    DataTracer
};

class vTracer : public QObject
{
    Q_OBJECT
public:



public:
    explicit vTracer(QCustomPlot *_plot,QCPGraph *_graph, TracerType _type);//这里与原贴不同，按照原贴构造总是过不去
    ~vTracer();

    void setLabelFont(QFont font);
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setText(const QString &text);
    void setLabelPen(const QPen &pen);
    void updatePosition(double xValue, double yValue);
    void setVisible(bool visible);

protected:
    QCustomPlot *plot ;	     //传入实例化的QcustomPlot
    QCPGraph *graph;	   	 //这里是存传入的绘图图层
    QCPItemTracer *tracer;   // 跟踪的点
    QCPItemText *label;   	 // 显示的数值
    QCPItemLine *arrow;  	 // 箭头

    TracerType type;
    bool visible;

signals:

public slots:

};

#endif // MYTRACER_H
