/*
 ************************************** Copyright ******************************
 * @FileName     :  vdebug.h
 * @Author       :  Seasky.Liu
 * @Date         :  2021/9/9
 * @Time         :  15:34
 * @Email        :  xxx.
 ******************************************************************************
 */
#ifndef VDEBUG_H
#define VDEBUG_H


#include <QString>

void doCritical(const QString &str);
void doWarning(const QString &str);
void doInfo(const QString &str);
void doAbout(const QString &str);

#endif // VDEBUG_H
