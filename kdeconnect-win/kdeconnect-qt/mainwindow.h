#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core\kclogger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	void displayDebugMessage(QtMsgType type, const QString &msg);
	
	void on_radioButtonLog_toggled(bool checked);
    void on_pushButtonMyName_clicked();
    void on_pushButtonUnPair_clicked();
    void on_pushButtonRefresh_clicked();
    void on_lineEditMyName_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
