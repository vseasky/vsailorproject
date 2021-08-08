#ifndef VCODECONVERTER_H
#define VCODECONVERTER_H
#include <QString>
#include <QTextCodec>
/*
 *QT的串口操作以及常见的编码格式为QByteArray
 *该文件提供常见编码格式的转换操作The encoding format
 */
//该串口调试助手支持的编码格式列表
typedef enum
{
    SerialCodeNo = 0,
    SerialCodeUtf8, //Utf8编码格式
    SerialCodeUtf16,    //Utf16编码格式
    SerialCodeGB18030,  //GBK编码格式、兼容GBK18030
    SerialCodeISO8859,  //IOS8859-1
    SerialCodeBig5,     //Big5
    SerialShiftJIS,
}vSerialCodeModeEnum;

//数据处理编码格式
//数据解码操作
QByteArray CodeDecoding(QByteArray const &qByteArr);
//数据编码操作
QByteArray CodeCoding(QByteArray const &qByteArr);

#endif // VCODECONVERTER_H
