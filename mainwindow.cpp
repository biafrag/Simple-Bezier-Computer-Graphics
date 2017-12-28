#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->openGLWidget,
     SIGNAL( updateMousePositionText(const QString&)),
     statusBar(),
     SLOT(showMessage(const QString&)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
