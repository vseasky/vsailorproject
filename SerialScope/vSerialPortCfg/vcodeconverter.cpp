#include "vcodeconverter.h"

vSerialCodeModeEnum vSerialCodeMode = SerialCodeUtf8;
//实际为编码格式解码
QByteArray CodeDecoding(QByteArray const &qByteArr)
{
    QString tmpQStr;
    switch (vSerialCodeMode)
    {
        case SerialCodeNo     :tmpQStr = qByteArr;break;
        case SerialCodeUtf8   :tmpQStr = QTextCodec::codecForName("UTF-8")->toUnicode(qByteArr);break;
        case SerialCodeUtf16  :tmpQStr = QTextCodec::codecForName("UTF-16")->toUnicode(qByteArr);break;
        case SerialCodeGB18030:tmpQStr = QTextCodec::codecForName("GB18030")->toUnicode(qByteArr);break;
        case SerialCodeISO8859:tmpQStr = QTextCodec::codecForName("ISO 8859-1")->toUnicode(qByteArr);break;
        case SerialCodeBig5   :tmpQStr = QTextCodec::codecForName("Big5")->toUnicode(qByteArr);break;
        case SerialShiftJIS   :tmpQStr = QTextCodec::codecForName("Shift-JIS")->toUnicode(qByteArr);
        default:;break;
    }
    return tmpQStr.toLocal8Bit();
};
//编码
QByteArray CodeCoding(QByteArray const &qByteArr)
{
    QByteArray tmpQByte;
    switch (vSerialCodeMode)
    {
        case SerialCodeNo     :tmpQByte = QTextCodec::codecForName("")->fromUnicode(qByteArr).replace("\n","\r\n");break;
        case SerialCodeUtf8   :tmpQByte = QTextCodec::codecForName("UTF-8")->fromUnicode(qByteArr).replace("\n","\r\n");break;
        case SerialCodeUtf16  :tmpQByte = QTextCodec::codecForName("UTF-16")->fromUnicode(qByteArr).replace("\n","\r\n");break;
        case SerialCodeGB18030:tmpQByte = QTextCodec::codecForName("GB18030")->fromUnicode(qByteArr).replace("\n","\r\n");break;
        case SerialCodeISO8859:tmpQByte = QTextCodec::codecForName("ISO 8859-1")->fromUnicode(qByteArr).replace("\n","\r\n");break;
        case SerialCodeBig5   :tmpQByte = QTextCodec::codecForName("Big5")->fromUnicode(qByteArr).replace("\n","\r\n");break;
        case SerialShiftJIS   :tmpQByte = QTextCodec::codecForName("Shift-JIS")->fromUnicode(qByteArr).replace("\n","\r\n");
        default:;break;
    }
    return tmpQByte;
};


