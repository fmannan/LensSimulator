#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "lensprofileview.h"
#include "core/lensanalysis.h"
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool loadPrescription(const QString& path);
    bool analyze();
    bool exportSvg(const QString& path);
private:
    Lens mLens;
    LensProfileView* mView;
    QDoubleSpinBox *mBeam, *mLower, *mUpper;
    QLabel* mResults;
    QPushButton* mAnalyze;
    QTableWidget* mTable;
};
#endif
