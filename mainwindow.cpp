#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <QFileDialog>
#include <QWheelEvent>
#include <QTableView>
#include <QtSvg>
#include <QSvgGenerator>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), mLensProfileView(&mLens, parent),
    ui(new Ui::MainWindow), mBoolPanningState(false)
{
    ui->setupUi(this);
    setCentralWidget(&mLensProfileView);
    ui->statusBar->showMessage(tr("Ready"));

    ui->dockWidgetEditorView->hide();
    ui->action_Editor->setChecked(false);
    ui->tableViewLensEditor->showGrid();

    ui->action_Console->setChecked(false);
    connect(ui->dockWidgetConsole, SIGNAL(closeEvent()), SLOT(closeConsole));
    ui->dockWidgetConsole->hide();
    //QStandardItemModel *model = new QStandardItemModel(2, 3, this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveSVG()
{  // TODO: ADD saveSVG() options from doc.qt.io/qt-5/qtsvg-svggenerator-example.html
	QString newPath = QFileDialog::getSaveFileName(this, tr("Save SVG"), mPath, tr("SVG files (*.svg)"));

	if (newPath.isEmpty())
		return;

	mPath = newPath;

	QSvgGenerator generator;
	generator.setFileName(mPath);
	generator.setSize(QSize(200, 200));
	generator.setViewBox(QRect(0, 0, 200, 200));
	generator.setTitle(tr("QtLensSimulator"));
	generator.setDescription(tr("QtLensSimulator created lens drawing"));

	QPainter painter;
	painter.begin(&generator);
	mLensProfileView.paint(painter);
	painter.end();
}

void MainWindow::on_action_Open_triggered()
{
    std::stringstream stream;
    ui->statusBar->showMessage(tr("Open Lens Prescription ..."), 1000);
    ui->textEditConsole->append(tr("Open Lens Prescription ..."));
    try {
        QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Lens Prescription"), "", tr("Lens Prescription Text File (*.txt)"));

        if(fileName.isEmpty())
           return;

        qDebug() << "Opening " << fileName << "\n";
        ui->textEditConsole->append(tr("Opening ") + fileName);
        mLens.load(fileName);
        mLensProfileView.loadLens(&mLens);  // call update so that lensprofileview displays the newly loaded lens
        ui->statusBar->showMessage(tr("done"), 1000);

        // TODO: Move this to some appropriate function that is tied to the user interface
        mLensProfileView.clearRayBundles(); // clear previous ray bundles if any
        mLensProfileView.setRayBundleDraw(true);
        //mLensProfileView.addRayBundle(mLens.raytraceRayBundleBackToFront( glm::vec4(0.0f, 0.0f, mLens.thickness(), 1.0f), 4 ), Colorf(1.0f, 0.0f, 0.0f));
//        mLensProfileView.addRayBundle(mLens.raytraceRayBundleBackToFront( glm::vec3(0.0f, 0.0f, 20 + mLens.thickness()), 3 ), Colorf(0.0f, 0.0f, 1.0f));
        //mLensProfileView.addRayBundle(mLens.raytraceRayBundleFrontToBack( glm::vec4(0.0f, 0.0f, -mLens.thickness(), 1.0f), 4 ), Colorf(0.0f, 0.0f, 1.0f));
//        mLensProfileView.addRayBundle(mLens.raytraceRayBundleFrontToBack( glm::vec4(0.0f, 0.0f, -mLens.thickness(), 0.0f), 4 ), Colorf(1.0f, 1.0f, 1.0f));
//        glm::vec3 pt;
////        mLensProfileView.addRayBundle(mLens.imagePoint( glm::vec4(0.0f, 0.0f, -1000.0f + mLens.thickness(), 1.0f), pt), Colorf(1.0f, 1.0f, 0.0f));
////        qDebug() << "image coord: " << pt.x << ", " << pt.y << ", " << pt.z;

//        mLensProfileView.addRayBundle(mLens.imagePoint( glm::vec4(0.0f, 0.0f, -mLens.thickness(), 0.0f), pt), Colorf(1.0f, 0.0f, 1.0f));
//        qDebug() << "image coord: " << pt.x << ", " << pt.y << ", " << pt.z;
//        mLensProfileView.addRayBundle(mLens.raytraceRayBundleFrontToBack( glm::vec3(0.0f, 0.0f, -2 * mLens.thickness()), 10 ), Colorf(0.0f, 1.0f, 0.0f));
//        mLensProfileView.addRayBundle(mLens.raytraceRayBundleFrontToBack( glm::vec3(0.0f, 100.0f, -2 * mLens.thickness()), 10 ), Colorf(0.0f, 1.0f, 1.0f));
        //mLensProfileView.addRayBundle(mLens.raytraceRayBundleIrisToBack( glm::vec4(0.0f, 16.0f, 0.0f, 1.0f), 4 ), Colorf(1.0f, 0.0f, 1.0f));
        //mLensProfileView.addRayBundle(mLens.raytraceRayBundleIrisToFront( glm::vec4(0.0f, 16.0f, 0.0f, 1.0f), 2 ), Colorf(0.0f, 1.0f, 1.0f));

        double zExitPupil, sizeExitPupil;
        RayBundle topAp, bottomAp;
        mLens.findExitPupil(zExitPupil, sizeExitPupil, topAp, bottomAp);
        mLensProfileView.addRayBundle(topAp, Colorf(1.0f, 0.0, 0.0));
        mLensProfileView.addRayBundle(bottomAp, Colorf(1.0f, 0.0, 0.0));

        double zEntrancePupil, sizeEntrancePupil;
        mLens.findEntrancePupil(zEntrancePupil, sizeEntrancePupil, topAp, bottomAp);
        mLensProfileView.addRayBundle(topAp, Colorf(0.0f, 0.0, 1.0));
        mLensProfileView.addRayBundle(bottomAp, Colorf(0.0f, 0.0, 1.0));

        glm::vec3 frontFocus, backFocus;
        double frontPP, backPP;
        RayBundle bundle;
        frontFocus = mLens.findFrontFocus(bundle, &frontPP);
        mLensProfileView.addRayBundle(bundle, Colorf(0.0f, 1.0f, 0.0f));

        qDebug() << "Front focus point : " << frontFocus.x << " , " << frontFocus.y << " , " << frontFocus.z;
        qDebug() << "Front Principal Plane : " << frontPP;

        stream << "\n\nSummary\n------------\n";
        stream << "Front focus point : " << frontFocus.x << " , " << frontFocus.y << " , " << frontFocus.z << "\n";
        stream << "Front Principal Plane (PP) : " << frontPP << "\n";
        stream << "Front Focus Distance (from PP) : " << frontPP - frontFocus.z << "\n";
        stream << "Entrance Pupil Position : " << zEntrancePupil << "\n";
        stream << "Entrance Pupil Size     : " << sizeEntrancePupil << "\n";
        stream << "Front Focus from EP : " << zEntrancePupil - frontFocus.z << "\n";

        backFocus = mLens.findBackFocus(bundle, &backPP);
        mLensProfileView.addRayBundle(bundle, Colorf(0.5f, 0.5f, 0.5f)); // Colorf(1.0f);

        qDebug() << "Back focus point : " << backFocus.x << " , " << backFocus.y << " , " << backFocus.z;
        qDebug() << "Back Principal Plane : " << backPP;

        stream << "Back focus point : " << backFocus.x << " , " << backFocus.y << " , " << backFocus.z << "\n";
        stream << "Back Principal Plane (PP'): " << backPP << "\n";
        stream << "Back Focus Distance (from PP') : " << backFocus.z - backPP << "\n";
        stream << "Exit Pupil Position  : " << zExitPupil << "\n";
        stream << "Exit Pupil Size      : " << sizeExitPupil << "\n";
        stream << "Back Focus from Exit Pupil : " << backFocus.z - zExitPupil << "\n";
        stream << "\n\n";
        stream << "First set of points (-1000, 10, 5)\n";
        auto pts = mLens.imagePlanarGridPoints1D(-1000, 10, 5);
        stream << "z : " << pts.begin()->first.z << " -> " << pts.begin()->second.z << "\n";
        qDebug() << "z : " << pts.begin()->first.z << " -> " << pts.begin()->second.z;
        for(auto it = pts.begin(); it != pts.end(); it++) {
            stream << "(" << it->first.z << ", " << it->first.y << ") -> (" << it->second.z << ", " << it->second.y << ")\n";
            qDebug() << "(" << it->first.z << ", " << it->first.y << ") -> (" << it->second.z << ", " << it->second.y << ")";
        }
        stream << "\n\nSecond set of points (-919.1883, 10, 5)\n";
        pts = mLens.imagePlanarGridPoints1D(-919.1883, 10, 5);
        stream << "z : " << pts.begin()->first.z << " -> " << pts.begin()->second.z << "\n";
        qDebug() << "z : " << pts.begin()->first.z << " -> " << pts.begin()->second.z;
        for(auto it = pts.begin(); it != pts.end(); it++) {
            stream << "(" << it->first.z << ", " << it->first.y << ") -> (" << it->second.z << ", " << it->second.y << ")\n";
            qDebug() << "(" << it->first.z << ", " << it->first.y << ") -> (" << it->second.z << ", " << it->second.y << ")";
        }

        stream << "\n\nThird set of points (-843.5500, 10, 5)\n";
        pts = mLens.imagePlanarGridPoints1D(-843.5500, 10, 5);
        stream << "z : " << pts.begin()->first.z << " -> " << pts.begin()->second.z << "\n";
        qDebug() << "z : " << pts.begin()->first.z << " -> " << pts.begin()->second.z;
        for(auto it = pts.begin(); it != pts.end(); it++) {
            stream << "(" << it->first.z << ", " << it->first.y << ") -> (" << it->second.z << ", " << it->second.y << ")\n";
            qDebug() << "(" << it->first.z << ", " << it->first.y << ") -> (" << it->second.z << ", " << it->second.y << ")";
        }
        ui->textEditConsole->append(QString::fromStdString(stream.str()));
    } catch(const std::exception& e) {
        ui->statusBar->showMessage(tr("Unable to open file"), 2000);
        qCritical() << e.what();
        ui->textEditConsole->append(tr("Unable to open file"));
        ui->textEditConsole->append(e.what());
    }
}

void MainWindow::wheelEvent(QWheelEvent  *event)
{
//    qDebug() << event->pixelDelta();
//    qDebug() << event->angleDelta();
//    qDebug() << event->orientation();
//    qDebug() << event->pos();
//    qDebug() << event->globalPos();
    if(event->angleDelta().y() > 0)
        mLensProfileView.zoomIn();
    else
        mLensProfileView.zoomOut();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << event->button();
    if(mBoolPanningState) {
        QPoint cPos = event->pos();
        QPoint diff = cPos - mPanLastPos;
//        qDebug() << cPos;
//        qDebug() << event->x() << " " << event->y();
//        qDebug() << diff;
        mLensProfileView.pan(diff.x(), -diff.y());
        mPanLastPos = cPos;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        mPanLastPos = event->pos();
        mBoolPanningState = true;
        //qDebug() << "drag start ... " << mPanLastPos;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        mPanLastPos = event->pos();
        mBoolPanningState = false;
        //qDebug() << "drag stopped. " << mPanLastPos;
    }
}

void MainWindow::on_action_Editor_toggled(bool arg1)
{
    qDebug() << arg1;
    if(arg1) {
        ui->dockWidgetEditorView->show();
    } else {
        ui->dockWidgetEditorView->hide();
    }
}

void MainWindow::on_action_Console_toggled(bool arg1)
{
    if(arg1) {
        ui->dockWidgetConsole->show();
        ui->dockWidgetConsole->dockLocationChanged(Qt::BottomDockWidgetArea);
    } else {
        ui->dockWidgetConsole->hide();
    }
}

void MainWindow::consoleClose(QCloseEvent *event)
{
    qDebug() << "consoleClose";
    ui->action_Console->setChecked(false);
}
