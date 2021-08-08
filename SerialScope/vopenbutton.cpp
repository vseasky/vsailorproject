#include "vopenbutton.h"
#include <QBitmap>

vOpenButton::vOpenButton(QWidget *parent) : QPushButton(parent)
{
    QPixmap pixmap(":/image/image/open1.png");
    pixmap = pixmap.scaled(64,24,Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
    this->setMask(pixmap.mask());
}
