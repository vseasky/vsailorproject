#ifndef VQWEBENGINEVIEW_H
#define VQWEBENGINEVIEW_H

#include <QWidget>
#include <QWebEngineView>

class vQWebEngineView : public QWidget
{
    Q_OBJECT
public:
    explicit vQWebEngineView(QWidget *parent = nullptr);
    ~vQWebEngineView(void);
public slots:
    void vQWebEngineView::vTextChanged(void);
protected:
    void resizeEvent(QResizeEvent *);
private:
    QWebEngineView* view;
signals:

};

#endif // VQWEBENGINEVIEW_H
