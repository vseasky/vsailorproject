#pragma once

#include <QObject>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

class CSVHelper : public QObject
{
	Q_OBJECT

public:
	CSVHelper(QObject *parent);
	~CSVHelper();
	
	static QList<QList<QString>> ReadCSVtoData(QString CSVFilePath);
	static int SaveCSVtoFile(QList<QList<QString>> CSVData, QString CSVFilePath);
	
	
private:
	
	static int CountOfQuotationMark(QString str);//计算字符串中英文引号的个数
	static bool isEven(int num);//计算num是否为偶数
	static QList<QString> ReadLineToCell(QString str);//将CSV文件的一行数据分解到单元格中
	static QString ReadDataToCell(QString str);//将CSV的一个单元格数据处理成最终的单元数据，即删除多余的引号

	static QList<QList<QString>> CheckCSVData(QList<QList<QString>> CSVData);//校验csv数据
	static QString CombineLine(QList<QString> lineData);//合并一行数据
	static QString TranData(QString str);//将数据进行转义
};
