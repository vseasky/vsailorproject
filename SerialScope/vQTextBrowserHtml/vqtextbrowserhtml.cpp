#include "vqtextbrowserhtml.h"
#include <windows.h>
#include <QTextCodec>
#include <QDesktopServices>
#include <QDebug>


//显示关于信息，作者信息
vQTextBrowserHtml::vQTextBrowserHtml(QWidget *parent) : QTextBrowser(parent)
{
    this->setOpenLinks(false);
    this->setAlignment(Qt::AlignHCenter);
    connect(this, &QTextBrowser::anchorClicked, this, &vQTextBrowserHtml::openHtml);
//    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
//    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
//                      SLOT(replyFinished(QNetworkReply *)));
//    manager->get(QNetworkRequest(QUrl("https://github.com/SEASKY-Master/Yolo-for-k210/blob/master/readme.md")));
}

void vQTextBrowserHtml::openHtml(const QUrl& url)
{
//    qDebug("open url: %s",url.toString().toStdString().data());
    QDesktopServices::openUrl(url);
}
void vQTextBrowserHtml::setInfo(QString name,QString mac)
{
    QString htmlSet =this->toHtml();
    htmlSet.replace("macNameADDR",name);
    htmlSet.replace("macIpADDR",mac);
    this->setHtml(htmlSet);
}

//void myQTextBrowserHtml::replyFinished(QNetworkReply * pReply)
//{
//        qDebug() << pReply->readAll();
//}
