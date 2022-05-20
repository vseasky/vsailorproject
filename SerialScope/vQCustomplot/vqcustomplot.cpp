#include "vqcustomplot.h"
#include <QDebug>
/*
 *****************************
 ************波形绘制************
 ******************************
 */

const uint8_t KEY_CTRL = 0X01;
const uint8_t KEY_ALT  = 0X02;
const uint8_t KEY_ALT_CTRL=KEY_CTRL|KEY_ALT;

vQCustomPlot::vQCustomPlot(QWidget *parent) : QCustomPlot(parent)
{
    this->setOpenGl(true);
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                          QCP::iSelectLegend | QCP::iSelectPlottables);
    this->axisRect()->setupFullAxesBox();
    this->xAxis2->setVisible(true);
    this->xAxis2->setTickLabels(false);
    this->yAxis2->setVisible(true);
    this->yAxis2->setTickLabels(false);

    // 使上下两个X轴的范围总是相等，使左右两个Y轴的范围总是相等
    connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)),
            this->xAxis2, SLOT(setRange(QCPRange)));
    connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)),
            this->yAxis2, SLOT(setRange(QCPRange)));
    //选中图例显示
    connect(this, SIGNAL(selectionChangedByUser()),
            this, SLOT(selectionChanged()));

    //双击修改坐标轴名称
    connect(this, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)),
            this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    //双击修改图例名称
    connect(this, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)),
            this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    // connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    connect(&GraphShowTimer,&QTimer::timeout,
            this,&vQCustomPlot::GraphShowSlot);
    //显示图例名称
    this->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    this->legend->setFont(legendFont);
    this->legend->setSelectedFont(legendFont);
    this->legend->setSelectableParts(QCPLegend::spItems);

    //追踪光标
    mxTracer = new vTracer(this, this->graph(),TracerType::DataTracer);
    //快捷键
    myPlotShortcut.X_Shortcut = Qt::Key_X;//横向缩放
    myPlotShortcut.Y_Shortcut = Qt::Key_V;//纵向缩放
    myPlotShortcut.All_Shortcut = Qt::Key_A;
    this->keyStatus = 0;
    this->xAxis->setRange(0,300);
    this->yAxis->setRange(0,1000);
    GraphShowTimer.setTimerType(Qt::PreciseTimer);
    TimerStop();
    //鼠标右键逻辑
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequest(QPoint)));
}
vQCustomPlot::~vQCustomPlot()
{
    delete  mxTracer;
}
void vQCustomPlot::TimerStart(void)
{
    TimerEnable = true;
    if(!GraphShowTimer.isActive())
    {
        GraphShowTimer.start(GraphShowTimerSet);
    }
}
void vQCustomPlot::TimerStop(void)
{
    TimerEnable = false;
    if(GraphShowTimer.isActive())
    {
        GraphShowTimer.stop();
    }
}
void vQCustomPlot::selectionChanged(void)
{
    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        this->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        this->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        this->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        this->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<this->graphCount(); ++i)
    {
        QCPGraph *graph = this->graph(i);
        QCPPlottableLegendItem *item = this->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}

void vQCustomPlot::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    // 双击轴标签，设置轴标签
    if (part == QCPAxis::spAxisLabel)
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this,
                                                 QString::fromLocal8Bit("更改坐标名称"),
                                                 QString::fromLocal8Bit("新的坐标名称"), QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            this->replot(QCustomPlot::rpQueuedReplot);
        }
    }
}

void vQCustomPlot::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    // 通过双击图形的图例项来重命名图形
    Q_UNUSED(legend)

    if (item)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName =
                QInputDialog::getText(this,
                                      QString::fromLocal8Bit("更改曲线名称"),
                                      QString::fromLocal8Bit("新的曲线名称"),
                                      QLineEdit::Normal, plItem->plottable()->name(),
                                      &ok,Qt::WindowCloseButtonHint);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            this->replot(QCustomPlot::rpQueuedReplot);
        }
    }
}
void vQCustomPlot::colorSet(void)
{
    QColor color = QColorDialog::getColor(Qt::red, this,
                                          QString::fromLocal8Bit("设置曲线颜色"),
                                          QColorDialog::ShowAlphaChannel);
    QPen graphPen;
    graphPen.setColor(color);
    graphPen.setWidthF(1.5);
    this->selectedGraphs().first()->setPen(graphPen);
}
/*鼠标键盘事件*/
void vQCustomPlot::wheelEvent(QWheelEvent *ev)
{
    //如果选择了轴，只允许缩放该轴的方向
    //如果没有选择轴，两个方向都可以缩放
    uint8_t key_val = this->keyStatus;
    if(key_val != KEY_ALT_CTRL)
    {
        if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
            this->axisRect()->setRangeZoom(this->xAxis->orientation());
        else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
            this->axisRect()->setRangeZoom(this->yAxis->orientation());
        else
            this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    }
    QCustomPlot::wheelEvent(ev);
}
void vQCustomPlot::mousePressEvent(QMouseEvent *ev)
{
    //如果选择了一个轴，只允许拖动该轴的方向
    //如果没有选择轴，两个方向都可以拖动
    uint8_t key_val = this->keyStatus;
    if(key_val != KEY_ALT_CTRL)
    {
        if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
            this->axisRect()->setRangeDrag(this->xAxis->orientation());
        else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
            this->axisRect()->setRangeDrag(this->yAxis->orientation());
        else
            this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    }
    QCustomPlot::mousePressEvent(ev);
}
//void vQCustomPlot::mouseReleaseEvent(QMouseEvent *ev)
//{
//    //鼠标按键
//    switch(ev->button())
//    {
//        case Qt::LeftButton:qDebug()<<"Release Left";break;
//        case Qt::RightButton:qDebug()<<"Release Right";break;
//        case Qt::MidButton:qDebug()<<"Release Mid";break;
//        default:break;
//    }

//    QCustomPlot::mouseReleaseEvent(ev);
//}
void vQCustomPlot::rescaleYAxis(void)
{
    this->yAxis->rescale(true);
    this->replot(QCustomPlot::rpQueuedReplot);
}
/*光标追踪数据点*/
void vQCustomPlot::mouseMoveEvent(QMouseEvent *ev)
{
    if(mxTracer==nullptr)
    {
        return;
    }
    if(this->selectedGraphs().size() <= 0)
    {
        mxTracer->setVisible(false);
        QCustomPlot::mouseMoveEvent(ev);
        return;
    }
    mxTracer->setVisible(true);
    //获取容器
    QSharedPointer<QCPGraphDataContainer> tmpContainer;
    tmpContainer = this->selectedGraphs().first()->data();
    //获取x,y轴坐标
    double x = 0;
    double y = 0;
    {
        //使用二分法快速查找所在点数据
        x = this->xAxis->pixelToCoord(ev->pos().x());
        int low = 0, high = tmpContainer->size();
        while(high > low)
        {
            int middle = (low + high) / 2;
            if(x < tmpContainer->constBegin()->mainKey() ||
                    x > (tmpContainer->constEnd()-1)->mainKey())
                break;

            if(x == (tmpContainer->constBegin() + middle)->mainKey())
            {
                y = (tmpContainer->constBegin() + middle)->mainValue();
                break;
            }
            if(x > (tmpContainer->constBegin() + middle)->mainKey())
            {
                low = middle;
            }
            else if(x < (tmpContainer->constBegin() + middle)->mainKey())
            {
                high = middle;
            }
            if(high - low <= 1)
            {   //差值计算所在位置数据
                y = (tmpContainer->constBegin()+low)->mainValue() + ( (x - (tmpContainer->constBegin() + low)->mainKey()) *
                                                                      ((tmpContainer->constBegin()+high)->mainValue() - (tmpContainer->constBegin()+low)->mainValue()) ) /
                        ((tmpContainer->constBegin()+high)->mainKey() - (tmpContainer->constBegin()+low)->mainKey());
                break;
            }

        }
    }
    //更新Tracer
    QString text = "X:" + QString::number(x, 'g', 6) + " Y:" + QString::number(y, 'g', 6);
    mxTracer->updatePosition(x, y);
    mxTracer->setText(text);
    /*重新显示*/
    QCustomPlot::mouseMoveEvent(ev);
}

//void vQCustomPlot::enterEvent(QEvent *ev)
//{
//    qDebug()<<"enterEvent";
//    QCustomPlot::enterEvent(ev);
//}

//void vQCustomPlot::leaveEvent(QEvent *ev)
//{
//    //鼠标位置
//    QString text =QString("<center><h1>Mouse leaveEvent</h1></center>");
//    qDebug()<<text;
//    QCustomPlot::leaveEvent(ev);
//}
void vQCustomPlot::keyPressEvent(QKeyEvent *ev)
{
    int key = ev->key();
    if(key == myPlotShortcut.X_Shortcut)
    {
        if(this->keyStatus==KEY_ALT_CTRL)
        {
            this->axisRect()->setRangeZoom(Qt::Horizontal);//设置缩放
            this->axisRect()->setRangeDrag(Qt::Horizontal);//设置拖动
        }
    }
    else if(key == myPlotShortcut.Y_Shortcut)
    {
        if(this->keyStatus==KEY_ALT_CTRL)
        {
            this->axisRect()->setRangeZoom(Qt::Vertical);
            this->axisRect()->setRangeDrag(Qt::Vertical);
        }
    }
    else if(key == myPlotShortcut.All_Shortcut)
    {
//        qDebug()<<"a";
    }
    if(key== Qt::Key_Control)
    {
        this->keyStatus |= KEY_CTRL;
    }
    if(key== Qt::Key_Alt)
    {
        this->keyStatus |= KEY_ALT;
    }
    QCustomPlot::keyPressEvent(ev);
}
//键盘松开触发事件
void vQCustomPlot::keyReleaseEvent(QKeyEvent *ev)
{
    int key = ev->key();
    if(key== Qt::Key_Control)
    {
        this->keyStatus &= 0XFF^KEY_CTRL;
    }
    if(key== Qt::Key_Alt)
    {
        this->keyStatus &= 0XFF^KEY_ALT;
    }
    if(key == myPlotShortcut.X_Shortcut)
    {
        if(this->keyStatus==KEY_ALT_CTRL)
        {
            this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);//设置缩放
            this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);//设置拖动
        }
    }
    else if(key == myPlotShortcut.Y_Shortcut)
    {
        if(this->keyStatus==KEY_ALT_CTRL)
        {
            this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
            this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
        }
    }
    else if(key == myPlotShortcut.All_Shortcut)
    {
//        qDebug()<<"a";
    }
    QCustomPlot::keyReleaseEvent(ev);
}

void vQCustomPlot::moveLegend(void)
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            this->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            //this->replot(QCustomPlot::rpQueuedReplot);
        }
    }
}
void vQCustomPlot::vSetNameAddr(QString * addr)
{
    this->vName=addr;
}
void vQCustomPlot::addRandomGraph(void)
{
    this->addGraph();
    if(this->vName[this->graphCount()-1]!=nullptr)
    {
        this->graph()->setName(this->vName[this->graphCount()-1]);
    }
    this->graph()->setLineStyle((QCPGraph::LineStyle::lsLine));
    this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc,1.325));//数据点
    QPen graphPen;
    graphPen.setColor(QColor(rand()%245+10, rand()%245+10, rand()%245+10));
    graphPen.setWidthF(1.3);
    this->graph()->setPen(graphPen);
    this->replot(QCustomPlot::rpQueuedReplot);
}
void vQCustomPlot::removeSelectedGraph(void)
{
    if (this->selectedGraphs().size() > 0)
    {
        this->removeGraph(this->selectedGraphs().first());
        this->replot(QCustomPlot::rpQueuedReplot);
    }
}

void vQCustomPlot::removeAllGraphs(void)
{
    this->clearGraphs();
    //  this->replot(QCustomPlot::rpQueuedReplot);
}
void vQCustomPlot::hideSelectedGraph()
{
    if (this->selectedGraphs().size() > 0)
    {
        //获取图像编号
        int index = 0;
        for(;index < this->graphCount(); index++)
        {
            if(this->graph(index)->name() == this->selectedGraphs().first()->name())
            {
                break;
            }
        }
        //可见性控制
        if(this->selectedGraphs().first()->visible())
        {
            this->selectedGraphs().first()->setVisible(false);
            this->legend->item(index)->setTextColor(Qt::gray);
        }
        else{
            this->selectedGraphs().first()->setVisible(true);
            this->legend->item(index)->setTextColor(Qt::black);
        }
        this->replot(QCustomPlot::rpQueuedReplot);
    }
}

void vQCustomPlot::hideAllGraph()
{
    int index = 0;
    for(index = 0;index < this->graphCount(); index++){
        this->graph(index)->setVisible(false);
        this->legend->item(index)->setTextColor(Qt::gray);
    }

    this->replot(QCustomPlot::rpQueuedReplot);
}

void vQCustomPlot::showAllGraph()
{
    int index = 0;
    for(;index < this->graphCount(); index++){
        this->graph(index)->setVisible(true);
        this->legend->item(index)->setTextColor(Qt::black);
    }
    this->replot(QCustomPlot::rpQueuedReplot);
}
bool vQCustomPlot::isAllGraphHide(void)
{
    int index = 0;
    for(;index < this->graphCount(); index++)
    {
        if(this->graph(index)->visible())
        {
            return false;
        }
    }
    return true;
}
bool vQCustomPlot::ishaveGraphHide(void)
{
    int index = 0;
    for(;index < this->graphCount(); index++)
    {
        if(!this->graph(index)->visible())
        {
            return true;
        }
    }
    return false;
}
void vQCustomPlot::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    if(this->legend->selectTest(pos,false) >= 0) // context menu on legend requested
    {
        menu->addAction(QString::fromLocal8Bit("移动到左上角"), this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction(QString::fromLocal8Bit("移动到右上角"), this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction(QString::fromLocal8Bit("移动到右下角"), this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction(QString::fromLocal8Bit("移动到左下角"), this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
        menu->addSeparator();
    }
    if(this->graphCount()==0)
    {
        menu->addAction(QString::fromLocal8Bit("自动添加多条曲线"), this, SLOT(addSeaskyGraph()));
    }
    else if (this->graphCount() > 0)
    {
        menu->addAction(QString::fromLocal8Bit("移除所有曲线"), this, SLOT(removeAllGraphs()));
        menu->addSeparator();
        menu->addAction(QString::fromLocal8Bit("AUTO"), this, SLOT(rescaleYAxis()));
        menu->addAction(QString::fromLocal8Bit("实时刷新"), this, [=]
        {
            xAxisAuto = true;
        });
        //选择了曲线
        if (this->selectedGraphs().size() > 0)
        {
            menu->addSeparator();
            menu->addAction(QString::fromLocal8Bit("设置曲线颜色"), this, SLOT(colorSet()));
            //所选曲线是否可见
            if(this->selectedGraphs().first()->visible())
            {
                menu->addAction(QString::fromLocal8Bit("隐藏所选曲线"), this, SLOT(hideSelectedGraph()));
            }
            else
            {
                menu->addAction(QString::fromLocal8Bit("显示所选曲线"), this, SLOT(hideSelectedGraph()));
            }
        }
        menu->addSeparator();
        //不是所有曲线被隐藏
        if(!isAllGraphHide())
        {
            menu->addAction(QString::fromLocal8Bit("隐藏所有曲线"), this, SLOT(hideAllGraph()));
        }
        //有曲线未被显示
        if(ishaveGraphHide())
        {
            menu->addAction(QString::fromLocal8Bit("显示所有曲线"), this, SLOT(showAllGraph()));
        }
    }
    menu->addSeparator();
    menu->addAction(QString::fromLocal8Bit("导入Csv"),
                    this, SLOT(vReadCsvData()));
    menu->addAction(QString::fromLocal8Bit("导出Csv"),
                    this, SLOT(vSaveCsvData()));

    menu->popup(this->mapToGlobal(pos));
}
void vQCustomPlot::addSeaskyGraph(void)
{
    showEnable = false;
    this->clearGraphs();
    for(int i=0;i<lenSeasky;i++)
    {
        addRandomGraph();
    }
    showEnable = true;
}
void vQCustomPlot::SeaskyScope(const QVariant &rxData)
{
    QVector<float> rxSeaskyScope = rxData.value< QVector<float> >();
    lenSeasky = 0;
    lenSeasky = rxSeaskyScope.size();
    if((showEnable)&&(lenSeasky<=24)&&(lenSeasky>0))
    {
        for (int i =0;(i<this->graphCount())&&(i<lenSeasky);i++)
        {
            this->graph(i)->addData(this->graph(0)->dataCount(),(double)rxSeaskyScope[i]);
        }
    }
}
void vQCustomPlot::GraphShowSlot(void)
{
    static int upper,lower,length,last_upper;
    upper = this->xAxis->range().upper;
    lower = this->xAxis->range().lower;
    length = upper - lower;
    if(last_upper>upper)
    {
        xAxisAuto=false;
    }
    else if(upper>=this->xAxis->range().upper)
    {
        xAxisAuto =true;
    }
    if(xAxisAuto == true)
    {
        this->yAxis->rescale(true);
        this->xAxis->rescale(true);
        upper = this->xAxis->range().upper;
        this->xAxis->setRange(upper,length,Qt::AlignRight);
    }
    last_upper = upper;
    this->replot(QCustomPlot::rpQueuedReplot);
    emit ledShowEnable();
}
void vQCustomPlot::showCsvData(double *data,int size)
{
    if(this->graphCount()==size)
    {
        for (int i =0;(i<size);i++)
        {
            this->graph(i)->addData(this->graph(0)->dataCount(),(double)data[i]);
        }
    }
    else
    {
        if(this->graphCount()==0)
        {
            for(int i=0;i<size;i++)
            {
                addRandomGraph();
            }
        }
    }
}
void vQCustomPlot::vSaveCsvData(void)
{
    QString dirpath = QFileDialog::getSaveFileName(this, QStringLiteral("保存为Csv"),
                                                   qApp->applicationDirPath(),
                                                   QString(tr("File (*.csv)")));
    if(dirpath!=NULL)
    {
        QFile file(dirpath);
        //方式：Append为追加，WriteOnly，ReadOnly
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(NULL,QString::fromLocal8Bit("提示"),
                                  QString::fromLocal8Bit("无法创建文件"));
            return ;
        }
        else
        {
            /*以下仅适用于key为int的数据*/
            int myGraphCount,posCount,maxSize,maxSizePos;
            QString str;
            QVector<double> saveData[24];
            myGraphCount = this->graphCount();
            /*将数据加载到QVector<double>，进行排列*/
            /*寻找数据最多的曲线*/
            maxSize = 0;
            posCount = 0;
            for(posCount = 0;posCount<myGraphCount;posCount++)
            {
                if(maxSize<this->graph(posCount)->dataCount())
                {
                    maxSize = this->graph(posCount)->dataCount();
                    maxSizePos = posCount;
                }
            }
            for(int i=0;i<24;i++)
            {
                /*分配指定大小的内存*/
                saveData[i].resize(maxSize+1);
            }
            for(posCount = 0;posCount<myGraphCount;posCount++)
            {
                for(int i = 0; i <this->graph(posCount)->dataCount(); i++)
                {
                    saveData[posCount][(int)(
                            this->graph(posCount)->data()->at(i)->key)] =
                            this->graph(posCount)->data()->at(i)->value;
                }
            }
            str.clear();
            str.append("time,");
            for(posCount=0;posCount<myGraphCount-1;posCount++)
            {
                str.append(this->graph(posCount)->name()+",");
            }
            str.append(this->graph(posCount)->name()+"\n");
            file.write(str.toStdString().c_str());//每次写入一行文本
            for(int i = 0; i < maxSize; i++)
            {
                str.clear();
                str.append(QString::number(i,10)+",");
                for(posCount=0;posCount<myGraphCount-1;posCount++)
                {
                    str.append(QString::number(saveData[posCount][i], 'f', 6)+",");
                }
                str.append(QString::number(saveData[posCount][i], 'f', 6)+"\n");
                file.write(str.toStdString().c_str());//每次写入一行文本
            }
            file.close();
        }
    }
}
void vQCustomPlot::vReadCsvData(void)
{
    QString dirpath = QFileDialog::getOpenFileName(this,QStringLiteral("导入Csv"),
                                                   qApp->applicationDirPath(), QString(tr("File (*.csv)")));
    if(dirpath!=NULL)
    {
        QFile file(dirpath);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(NULL,
                                  QString::fromLocal8Bit("提示"),
                                  QString::fromLocal8Bit("无法打开该文件"));
            return ;
        }
        else
        {
            QStringList list;
            static double csvData[24];
            list.clear();
            QTextStream csvStream(&file);
            //遍历行
            this->clearGraphs();
            this->replot(QCustomPlot::rpQueuedReplot);

            QString fileLine = csvStream.readLine();
            list =  fileLine.split(",",
                                  QString::SkipEmptyParts);
            for(int j=1;j<list.size();j++)
            {
                this->vName[j-1] = list[j];
                addRandomGraph();
            }
            for(int i = 0;!csvStream.atEnd();i++)
            {
                QString fileLine = csvStream.readLine();
                list =  fileLine.split(",",
                                       QString::KeepEmptyParts);
                //根据","开分隔开每行的列
                for(int j=1;j<list.size();j++)
                {
                    csvData[j-1] = list[j].toDouble();
                }
                this->showCsvData(csvData,
                                  list.size()-1);
            }
            file.close();
        }
    }
}
