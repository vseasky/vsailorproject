#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



struct MainWindowPrivate;


/**
 * Simple main window for demo
 */
class CMainWindow : public QMainWindow
{
	Q_OBJECT
private:
	MainWindowPrivate* d;///< private data - pimpl
	friend struct MainWindowPrivate;

protected:
	virtual void closeEvent(QCloseEvent* event) override;

public:
        // QAction *actionExit;
        // QAction *actionSaveState;
        // QAction *actionRestoreState;
        // QWidget *centralWidget;
        // QStatusBar *statusBar;
        // QMenuBar *menuBar;
        // QMenu *menuFile;
        // QMenu *menuView;
        // QMenu *menuAbout;
        // QMenu *menuTests;
        // QToolBar *toolBar;
  
	explicit CMainWindow(QWidget *parent = 0);
	virtual ~CMainWindow();

private slots:
	void on_actionSaveState_triggered(bool);
	void on_actionRestoreState_triggered(bool);
	void savePerspective();
	void onViewToggled(bool Open);
	void onViewVisibilityChanged(bool Visible);
	void createEditor();
	void createTable();
	void onEditorCloseRequested();
	void onImageViewerCloseRequested();
	void showStatusDialog();
	void toggleDockWidgetWindowTitle();
	void applyVsStyle();
	void createImageViewer();
};

#endif // MAINWINDOW_H
