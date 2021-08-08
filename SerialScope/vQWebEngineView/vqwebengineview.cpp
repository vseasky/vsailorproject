#include "vqwebengineview.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
vQWebEngineView::vQWebEngineView(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout * vQVBoxLayout = new QVBoxLayout(this);
    QWidget     * vQWidget = new QWidget(this);
    QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);

    QPushButton * vPreviousStep = new QPushButton(vQWidget);
    QPushButton * vNextStep = new QPushButton(vQWidget);
    QPushButton * vRefresh = new QPushButton(vQWidget);
    QLineEdit   * vHtmlAddr = new QLineEdit(vQWidget);
    vPreviousStep->setText("Previous");
    vNextStep->setText("Next");
    vRefresh->setText("Refresh");

    vQHBoxLayout->addWidget(vPreviousStep);
    vQHBoxLayout->addWidget(vNextStep);
    vQHBoxLayout->addWidget(vHtmlAddr);
    vQHBoxLayout->addWidget(vRefresh);
    vQHBoxLayout->setMargin(3);
    vQHBoxLayout->setContentsMargins(3,0,3,0);
    vQWidget->setAutoFillBackground(true);
    vQWidget->setStyleSheet("background-color:#F5F5F5;");
    vQWidget->setLayout(vQHBoxLayout);
    vQWidget->setMinimumSize(0,48);
    vQWidget->setMaximumSize(16777215,48);//宽度不限制
    QWidget  * vQWidget2 = new QWidget(this);
    view = new QWebEngineView(vQWidget2);
    vQWidget2->setAutoFillBackground(true);
    //view->page()->settings()->setAttribute(QWebEngineSettings::ShowScrollBars,false);
    view->load(QUrl("https://seasky-master.github.io/html/"));
    view->showMaximized();
    vQVBoxLayout->addWidget(vQWidget);
    vQVBoxLayout->addWidget(vQWidget2);
    this->setLayout(vQVBoxLayout);
    connect(vPreviousStep,&QPushButton::released,[=]()
    {
        view->page()->triggerAction(QWebEnginePage::Back);
    });
    connect(vNextStep,&QPushButton::released,[=]()
    {
        view->page()->triggerAction(QWebEnginePage::Forward);
    });
    connect(vRefresh,&QPushButton::released,[=]()
    {
        QString vhtmlStr;
        vhtmlStr = vHtmlAddr->text();
        if(vhtmlStr.indexOf("://")==-1)
        {
            vhtmlStr = "http://" + vhtmlStr;
        }
        view->load(QUrl(vhtmlStr));
    });
    connect(view,&QWebEngineView::loadFinished,[=](bool loadIt)
    {
        if(loadIt)
        {
            vHtmlAddr->setText(view->url().toString());
        }
        else
        {
            view->load(QUrl("https://seasky-master.github.io/html/"));
            //此处可以加载本地说明文档
        }
    });
    connect(vHtmlAddr,&QLineEdit::editingFinished,[=]()
    {
        QString vhtmlStr;
        vhtmlStr = vHtmlAddr->text();
        if(vhtmlStr.indexOf("://")==-1)
        {
            vhtmlStr = "http://" + vhtmlStr;
        }
        view->load(QUrl(vhtmlStr));
    });
}
vQWebEngineView::~vQWebEngineView(void)
{
    delete view;
}
void vQWebEngineView::resizeEvent(QResizeEvent *)
{
    view->resize(this->size());
}

