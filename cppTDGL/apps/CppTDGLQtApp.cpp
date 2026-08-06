#include "ApplicationInfo.hpp"

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);

    application.setApplicationName(QString::fromStdString(cppTDGL::ApplicationInfo::applicationName()));
    application.setOrganizationName(QString::fromStdString(cppTDGL::ApplicationInfo::organizationName()));

    QMainWindow mainWindow;
    mainWindow.setWindowTitle(QString::fromStdString(cppTDGL::ApplicationInfo::defaultWindowTitle()));
    mainWindow.resize(1200, 800);

    QMenu* fileMenu = mainWindow.menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("New Project"));
    fileMenu->addAction(QStringLiteral("Open Project"));
    fileMenu->addAction(QStringLiteral("Save Project"));
    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction(QStringLiteral("Exit"));
    QObject::connect(exitAction, &QAction::triggered, &application, &QApplication::quit);

    QMenu* simulationMenu = mainWindow.menuBar()->addMenu(QStringLiteral("&Simulation"));
    simulationMenu->addAction(QStringLiteral("Run"));
    simulationMenu->addAction(QStringLiteral("Cancel"));

    QMenu* helpMenu = mainWindow.menuBar()->addMenu(QStringLiteral("&Help"));
    helpMenu->addAction(QStringLiteral("About CppTDGL"));

    QLabel* centralLabel = new QLabel(QStringLiteral(
        "CppTDGL Qt application shell\n\n"
        "Current shell scope:\n"
        "  - project open/save menu placeholders\n"
        "  - simulation menu placeholders\n"
        "  - central workspace placeholder\n\n"
        "Core simulation, serialization, and CLI smoke workflows are implemented in the C++ library."
    ));
    centralLabel->setAlignment(Qt::AlignCenter);
    mainWindow.setCentralWidget(centralLabel);

    mainWindow.statusBar()->showMessage(QStringLiteral("Ready"));
    mainWindow.show();

    return application.exec();
}
