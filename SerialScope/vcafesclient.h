#ifndef VCAFESCLIENT_H
#define VCAFESCLIENT_H

#include <QObject>

class vCafesClient : public QObject
{
    Q_OBJECT
public:
    explicit vCafesClient(QObject *parent = nullptr);
    QString  get_localmachine_name();
    QString  get_localmachine_ip();
    QString  get_localmachine_mac();
signals:

};

#endif // MYCAFESCLIENT_H
