#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "core/kclogger.h"
#include "core/kdeconnectconfig.h"
#include "core/daemon.h"
#include "interfaces/devicesmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void displayDebugMessage(QtMsgType type, const QString &prefix, const QString &msg);

signals:
	void getQcaInfo();

private slots:
//	void showDeviceIdentity(const QString &device);
//	void displayError(int socketError, const QString &message);
//	void displayStatus(QString status);

	void on_radioButtonLog_toggled(bool checked);
    void on_pushButtonMyName_clicked();
    void on_pushButtonUnPair_clicked();
    void on_pushButtonRefresh_clicked();
	void on_lineEditMyName_textChanged();
	void on_pushButtonQcaInfo_clicked();
	void on_pushButtonSettingInfo_clicked();
	void on_listViewDevice_clicked(QModelIndex index);
	void on_dataChanged(QModelIndex tl, QModelIndex br);

private:
    Ui::MainWindow *ui;
	Daemon* daemon;
	DevicesModel* m_dmodel;
	const QString& mPrefix = "MainWindow";
};

#endif // MAINWINDOW_H
