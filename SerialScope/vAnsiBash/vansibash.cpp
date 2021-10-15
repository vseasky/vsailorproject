
#include "vansibash.h"
#include <QDebug>
vAnsiBash::vAnsiBash(QObject *parent) : QObject(parent)
{

}

//只解析字符串的Ansi终端控制颜色，而且只解析简单颜色
void  vAnsiBash::vAnsiColor(QByteArray *str,bool *Convertio)
{
    if(*Convertio)
    {
        return;
    }
    bool haveEsc = false;
    bool haveEnd = false;
    QByteArray vAnsiCode;
    int index_start = -1;
    int index_end   = -1;
    int vAnsiCodeLength = -1;
    //ansi_code开始下标 \x1b[
    index_start = str->indexOf(this->vEsc);
    //index=-1表示未查询到
    if(index_start>=0)
    {
        haveEsc = true;
        //ansi_code结束下标 m
        index_end = str->indexOf(this->vEnd,index_start);
        vAnsiCode.clear();
        if(index_end>index_start)
        {
            haveEnd = true;
            vAnsiCodeLength = index_end-(index_start+this->vEsc.size());
            //提取ansi_code
            vAnsiCode.append(str->mid(index_start+this->vEsc.size(),vAnsiCodeLength));
            QByteArrayList vAnsiCodelist = vAnsiCode.split(';');
            //获取颜色配置参数
            str->replace(index_start,index_end-index_start+1,ColorConfig(vAnsiCodelist));
            //继续处理下一段
            vAnsiColor(str,Convertio);
        }
        else
        {
            haveEsc = false;
            haveEnd = false;
        }
    }
    else
    {
        *Convertio = true;
    }
    if(haveEsc==false|haveEnd==false)
    {
        *Convertio = true;
    }
}

QByteArray  vAnsiBash::ColorConfig(const QByteArrayList &strlist)
{
    //只关心颜色
    QByteArray vAnsiColor = "<font";
    bool       isSetColor=false;
    bool       isClear   =false;
    uint ansi_code;
    for(int i=0;i<strlist.size();i++)
    {
        ansi_code = strlist.at(i).toUInt();
        switch (ansi_code)
        {
        case 0://关闭所有属性
            vAnsiColor = "</font>";
            isSetColor = false;
            isClear    = true;
            break;
        case 1://加粗
            vAnsiColor.append(" font-weight=\"bold\"");
            break;
        case 3://背景
            break;
        case 4://下划线
            break;
        case 9://划掉的
            break;
        case 22://正常的颜色或强度
            break;
        case 23://不是斜体，不是弗拉克图尔
            break;
        case 24://划掉下划线
            break;
        case 29://没有划掉
            break;
        case 39://前景颜色
            break;
        case 49://背景颜色
            break;
        case 38://设置前景颜色
        default:
        {
            //前景色代码
            if ((30 <= ansi_code && 37 >= ansi_code) ||//30~37
                (90 <= ansi_code && 97 >= ansi_code))  //90~97
            {
                isClear    = false;
                isSetColor = true;
                vAnsiColor.append(" color=\""+decodeColorBasic(ansi_code)+"\"");
            }
            else
            {
                //
            }
        };break;
        }
    }
    if(isClear)
    {
        return vAnsiColor;
    }
    if(!isSetColor)
    {
        vAnsiColor.append(defaultColor);
    }
    vAnsiColor.append(">");
    return vAnsiColor;
}
QByteArray vAnsiBash::decodeColorBasic(uint color_code)
{
    if(color_code-30 < 8)
    {
        color_code = color_code-30;
    }
    else if(color_code-40 < 8)
    {
        color_code = color_code-40;
    }
    else if(color_code-90 < 8)
    {
        color_code = color_code - 90 + 8;
    }
    else if(color_code-100 < 8)
    {
        color_code = color_code - 100 + 8;
    }
    else
    {
        return defaultColor;
    }
    return colors_basic[color_code];
}
