#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("LensSimulator");
    QApplication::setApplicationVersion("0.2.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("Lens prescription viewer and focus optimizer");
    parser.addHelpOption(); parser.addVersionOption();
    parser.addPositionalArgument("prescription", "Optional lens prescription file", "[prescription]");
    parser.process(app);
    if (parser.positionalArguments().size() > 1) parser.showHelp(2);
    MainWindow window;
    if (!parser.positionalArguments().isEmpty()) window.loadPrescription(parser.positionalArguments().front());
    else {
        const auto sample = QApplication::applicationDirPath() + "/LensPrescription/lens0.txt";
        if (QFileInfo::exists(sample)) window.loadPrescription(sample);
    }
    window.show();
    return app.exec();
}
