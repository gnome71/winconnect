#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/kdeconnectconfig.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->plainTextEditDebug->setHidden(true);
    ui->radioButtonLog->setChecked(false);

    //KdeConnectConfig* config = new KdeConnectConfig();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_radioButtonLog_toggled(bool checked)
{
    if(checked)
        ui->plainTextEditDebug->setHidden(false);
    else
        ui->plainTextEditDebug->setHidden(true);
}

void MainWindow::on_pushButtonMyName_clicked()
{

}

void MainWindow::on_pushButtonUnPair_clicked()
{

}

void MainWindow::on_pushButtonRefresh_clicked()
{

}

void MainWindow::on_lineEditMyName_textChanged(const QString &arg1)
{

}

