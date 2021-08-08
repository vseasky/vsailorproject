#ifndef VQTEXTBROWSERHTML_H
#define VQTEXTBROWSERHTML_H

#include <QTextBrowser>
#include <QString>
//网络类
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QNetworkInterface>
class vQTextBrowserHtml : public QTextBrowser
{
    Q_OBJECT
public:
    explicit vQTextBrowserHtml(QWidget *parent = nullptr);
    void openHtml(const QUrl& url);
    void setInfo(QString name,QString mac);
//    void anchorClickedSlot(const QUrl& url);
//    QNetworkAccessManager *manager;
//private slots:
//    void replyFinished(QNetworkReply *);
signals:

};

#endif // MYQTEXTBROWSERHTML_H
