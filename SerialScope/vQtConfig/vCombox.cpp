#include "vCombox.h"
#include<QComboBox>
#include<QListView>
#include<QDebug>

struct Student
{
    QString name;
    int     age;
};
//声明自定义类型,使得Qt能找到此类型的定义
Q_DECLARE_METATYPE(Student);
vCombox::vCombox(QWidget *parent):QComboBox(parent)
{
   this->setView(new  QListView());
   this->setCurrentIndex(0);//默认选择第一个
}

