/*
 ************************************** Copyright ******************************
 * @FileName     :  vdebug.cpp
 * @Author       :  Seasky.Liu
 * @Date         :  2021/9/9
 * @Time         :  15:34
 * @Email        :  xxx.
 ******************************************************************************
 */
#include "vdebug.h"
#include <QMessageBox>

#ifdef  VDEBUG_OPEN
#include <QDebug>
#endif

/*危险*/
void doCritical(const QString &str)
{
#ifdef VDEBUG_OPEN
    qDebug()<<"Critical:"<<str;
#endif
#ifdef vQMessageBox
    QMessageBox::critical(NULL,"Critical",str);
#endif
}
/*警告*/
void doWarning(const QString &str)
{
#ifdef VDEBUG_OPEN
    qDebug()<<"Warning:"<<str;
#endif
#ifdef vQMessageBox
    QMessageBox::warning(NULL,"Warning",str);
#endif
}
/*信息框*/
void doInfo(const QString &str)
{
#ifdef VDEBUG_OPEN
    qDebug()<<"Info:"<<str;
#endif
#ifdef vQMessageBox
    QMessageBox::information(NULL,"Info","");
#endif
}
/*关于*/
void doAbout(const QString &str)
{
#ifdef VDEBUG_OPEN
    qDebug()<<"About:"<<str;
#endif
#ifdef vQMessageBox
    QMessageBox::about(NULL,"About","作者：SEASKY-刘威\n"
                                  "参考：\n");
#endif
}
