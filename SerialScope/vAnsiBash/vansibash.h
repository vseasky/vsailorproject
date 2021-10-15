#ifndef VANSIBASH_H
#define VANSIBASH_H

#include <QObject>

class vAnsiBash : public QObject
{
    Q_OBJECT
public:
    explicit vAnsiBash(QObject *parent = nullptr);
    void     vAnsiColor(QByteArray *str,bool *Convertio);
    QByteArray ColorConfig(const QByteArrayList &strlist);
    QByteArray decodeColorBasic(uint color_code);
private:
    const QByteArray vEsc         = "\x1B[";
    const QByteArray vEnd         = "m";
    const QByteArray defaultColor ="#0000DF";
    //颜色代码
    const QByteArray colors_basic[16] =
    {
        "#000000",//黑
        "#0000FF",//蓝
        "#39B54A",//绿
        "#19CAAD",//水绿?
        "#F4606C",//红
        "#762671",//紫
        "#FF7F00",//黄
        "#CCCCCC",//白
        "#808080",//灰
        "#3232CD",//淡蓝
        "#00FF7F",//淡绿
        "#32CD32",//淡水绿
        "#FF2400",//淡红
        "#FF00FF",//淡紫
        "#DBDB70",//淡黄
        "#FFFFFF" //亮白
    };
signals:
};

#endif // VANSIBASH_H
