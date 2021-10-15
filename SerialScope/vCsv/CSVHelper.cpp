#include "CSVHelper.h"

QString QuotationMark = "\"";

CSVHelper::CSVHelper(QObject *parent)
	: QObject(parent)
{


}

CSVHelper::~CSVHelper()
{
}

//从CSV文件中读取数据
QList<QList<QString>> CSVHelper::ReadCSVtoData(QString CSVFilePath)
{
	//定义一个字符串矩阵，用于存储CSV文件中的数据
	QList<QList<QString>> CSVData;

	QFile CSVFile(CSVFilePath);
	if (!CSVFile.exists())
	{
		return CSVData;
	}
	//使用QFile类打开文件，使用流方式逐行读取
	if (CSVFile.open(QIODevice::ReadOnly))
	{
		//临时字符串，用于合并不完整的数据
		QString tempstring = "";
		//使用流逐行读取数据
		QTextStream stream(&CSVFile);
		//stream.setDevice(&CSVFile);
		while (!stream.atEnd())
		{
			//读取一行数据
			QString CSVLine = stream.readLine();
			//将数据与临时字符串合并
			if (tempstring == "")
			{
                tempstring = CSVLine;
                //添加一个标准的结束
                tempstring.append(",end");
			}
			else
			{
				tempstring = tempstring + "\r\n" + CSVLine;
			}
			//判断临时字符串中的英文引号个数是否为偶数，如果是偶数，则说明数据读取完整，如果不是则返回循环
			if (!isEven(CountOfQuotationMark(tempstring)))
			{
				continue;
			}
			CSVData.append(ReadLineToCell(tempstring));
			tempstring = "";
		}
		CSVFile.close();
	}
	return CSVData;
}

//将一行数据拆分成单元
QList<QString> CSVHelper::ReadLineToCell(QString str)
{
	//定义一个一维字符数组，用于存储拆分后的数据
	QList<QString> cell;
	//临时存储字符串，用于分析
	QString tempStr = "";
	//如果字符串以英文逗号结尾，说明最后还有一个空单元格，先做空单元格标记
	bool lastCellExist = false;
	if (str.endsWith(","))
	{
		lastCellExist = true;
	}
	//遍历字符串
	while (str.length() > 0)
	{
		//表示字符串中逗号的位置
		int index = -1;
		//查找第一个英文逗号的位置
		index = str.indexOf(',');
		//如果index不为-1，则说明存在英文逗号，进行判断
		if (index != -1)
		{
			//将字符串进行分割，英文逗号前的（含英文逗号）截取并附加到临时字符串后
			tempStr = tempStr + str.mid(0, index + 1);
			str = str.mid(index + 1);
			//如果临时字符串中的英文引号个数是偶数，则说明这个单元格内容完整，如果是奇数则返回循环
			if (isEven(CountOfQuotationMark(tempStr)))
			{
				//将信息进行处理并保持至单元格中
				cell.append(ReadDataToCell(tempStr));
				//将临时字符串清空
				tempStr = "";
			}
		}
		else
		{
			cell.append(ReadDataToCell(str));
			break;
		}
	}
	if (lastCellExist)
	{
		cell.append("");
	}
	return cell;
}

//将CSV的一个单元格数据处理成最终的单元数据，即删除多余的引号
QString CSVHelper::ReadDataToCell(QString str)
{
	//如果字符串是以逗号结尾的，则去除该逗号
	if (str.endsWith(","))
	{
		str = str.mid(0, str.length() - 1);
	}
	//如果字符串开头和结尾都有英文引号，则将这两个英文引号都去掉
	if (str.endsWith("\"") && str.startsWith("\""))
	{
		str = str.mid(1, str.length() - 2);
	}
	//用于保存处理的结果
	QString cellStr = "";
	//去除冗余的英文引号	
	str = str.replace("\"\"", "\"");
	return str;
}

//判断一个整数是否为偶数，如果是偶数则为true，否则为false。
bool CSVHelper::isEven(int num)
{
	//对2求余
	if (num % 2)
		return false;
	else
		return true;
}

//查找指定字符串中有多少个英文引号
int CSVHelper::CountOfQuotationMark(QString str)
{
	//英文引号的个数
	int count = 0;
	//逐个字符进行比较
	while (str.length() > 0)
	{
		//位置标识符
		int index = -1;
		//字符串中第一个英文引号的位置
		index = str.indexOf(QuotationMark);
		//如果位置标识符不是-1，则说明字符串中包含英文引号
		if (index != -1)
		{
			//剪掉包括这个英文引号之前的字符
			str = str.mid(index + 1);
			//将计数器加一
			count++;
		}
		else
			break;
	}
	return count;
}

//将数据保存成csv文件，如果传入的路径文件已存在，则返回-1，如果保存过程中出现错误，则返回0，保存成功返回1
int CSVHelper::SaveCSVtoFile(QList<QList<QString>> CSVData, QString CSVFilePath)
{
	//检测保存路径文件是否已经存在或者路径为空，如果存在返回-1
	if (CSVFilePath == "" || CSVFilePath == NULL)
		return -1;
	
	//如果文件存在，则返回-1
	QFile CSVFile(CSVFilePath);
	if (CSVFile.exists())
	{
		return -1;
	}
	//创建csv文件，用读写方式打开
	if (CSVFile.open(QIODevice::ReadWrite))
	{
		//校验数据
		CSVData = CheckCSVData(CSVData);
		//逐行进行录入
		for (int i = 0; i < CSVData.length(); i++)
		{
			//将一行数据进行合并
			QString str = CombineLine(CSVData[i]) + "\r\n";
			//写入数据
			CSVFile.write(str.toUtf8());			
		}
		//关闭文件
		CSVFile.close();
		return 1;
	}
	return 0;
}

//将数据进行转义
QString CSVHelper::TranData(QString str)
{
	//如果字符串中含有英文引号，则要进行转义，即用两个英文引号表示一个英文引号
	str.replace("\"", "\"\"");
	//如果字符串中包含英文逗号或英文引号或换行符，则要字符串两端添加英文引号
	if (str.contains(",") || str.contains("\r\n") || str.contains("\n") || str.contains("\""))
	{
		str = "\"" + str + "\"";
	}
	return str;
}

//合并一行数据
QString CSVHelper::CombineLine(QList<QString> lineData)
{
	QString str = "";
	//将一行数据中的各单元格合并，单元格数据进行转义，之间用英文逗号隔开
	for (int i = 0; i < lineData.length(); i++)
	{
		if (str == "")
			str = TranData(lineData[i]);
		else
			str = str + "," + TranData(lineData[i]);
	}
	return str;
}

//校验csv数据，csv数据应是整齐的，因此需要校验，并将缺少的单元用""补齐
QList<QList<QString>> CSVHelper::CheckCSVData(QList<QList<QString>> CSVData)
{
	//寻找数据行中单元格最大数
	int maxLength = 0;
	for (int i = 0; i < CSVData.length(); i++)
	{
		if (maxLength < CSVData[i].length())
			maxLength = CSVData[i].length();
	}
	//将单元格补齐
	for (int i = 0; i < CSVData.length(); i++)
	{
		if (CSVData[i].length() < maxLength)
		{
			for (int j = 0; j < maxLength - CSVData[i].length(); j++)
			{
				CSVData[i].append("");
			}
		}
	}
	return CSVData;
}
