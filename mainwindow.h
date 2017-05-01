#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "lensprofileview.h"
#include "lens.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	void saveSVG();

private slots:
    void on_action_Open_triggered();
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void on_action_Editor_toggled(bool arg1);

    void on_action_Console_toggled(bool arg1);
    void consoleClose(QCloseEvent* );

private:
    Ui::MainWindow *ui;
    LensProfileView mLensProfileView;
    Lens mLens;

    QPoint mPanLastPos;
    bool   mBoolPanningState;   // if true then scene pans as the mouse moves

	QString mPath;
};

#endif // MAINWINDOW_H
