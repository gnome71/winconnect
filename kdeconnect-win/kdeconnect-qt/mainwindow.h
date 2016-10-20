#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "core/kclogger.h"
#include "core/kdeconnectconfig.h"
//#include "core/networkpackage.h"
#include "core/udplistener.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
	void getQcaInfo();

private slots:
	void displayDebugMessage(QtMsgType type, const QString &prefix, const QString &msg);
	
	void showDeviceIdentity(const QString &device);
	void displayError(int socketError, const QString &message);
	void displayStatus(QString status);

	void on_radioButtonLog_toggled(bool checked);
    void on_pushButtonMyName_clicked();
    void on_pushButtonUnPair_clicked();
    void on_pushButtonRefresh_clicked();
	void on_lineEditMyName_textChanged();
	void on_pushButtonQcaInfo_clicked();
	void on_pushButtonSettingInfo_clicked();

private:
    Ui::MainWindow *ui;
	KdeConnectConfig* config;
	//UdpListenerThread udpListener;
};

#endif // MAINWINDOW_H
