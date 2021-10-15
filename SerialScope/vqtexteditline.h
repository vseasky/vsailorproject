#ifndef VQTEXTEDITLINE_H
#define VQTEXTEDITLINE_H

#include <QTextEdit>
#include <QKeyEvent>
class vQTextEditLine : public QTextEdit
{
    Q_OBJECT
public:
    explicit vQTextEditLine(QTextEdit *parent = nullptr);

public slots:
    void keyPressEvent(QKeyEvent *event);   //键盘按下事件
signals:

};

#endif // VQTEXTEDITLINE_H
