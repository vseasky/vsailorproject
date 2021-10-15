#include "vqtexteditline.h"
#include <QDebug>
vQTextEditLine::vQTextEditLine(QTextEdit *parent) : QTextEdit(parent)
{
    this->installEventFilter(this);
}
void vQTextEditLine::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        qDebug()<<"keyPressEvent";
    }
    event->accept();
}
