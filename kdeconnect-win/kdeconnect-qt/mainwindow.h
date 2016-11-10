/**
 * Copyright 2016 by Alexander Kaspar <alexander.kaspar@gmail.com>
 * based on the work from Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "core/kclogger.h"
#include "core/kdeconnectconfig.h"
#include "core/daemon.h"
#include "interfaces/devicesmodel.h"


namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void displayDebugMessage(QtMsgType type, const QString &prefix, const QString &msg);	//! Log message to GUI

signals:
	void getQcaInfo();

private slots:
//	void showDeviceIdentity(const QString &device);
//	void displayError(int socketError, const QString &message);
//	void displayStatus(QString status);

	void createTrayIcon();
	void createTrayActions();
	void showMessage(const QString& dev, const QString& msg);
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void messageClicked();

	void on_radioButtonLog_toggled(bool checked);
    void on_pushButtonMyName_clicked();
    void on_pushButtonUnPair_clicked();
    void on_pushButtonRefresh_clicked();
	void on_lineEditMyName_textChanged();
	void on_pushButtonQcaInfo_clicked();
	void on_pushButtonSettingInfo_clicked();
	void on_listViewDevice_clicked(QModelIndex index);
	void on_dataChanged(QModelIndex tl, QModelIndex br);
	void on_pushButtonOk_clicked();

private:
    Ui::MainWindow *ui;
	QSystemTrayIcon *trayIcon;
	Daemon* m_daemon;
	Device* m_currentDevice;
	DevicesModel* m_dmodel;
	QModelIndex m_currentIndex;
	KcLogger* m_logger;
	KdeConnectConfig* m_config;

	// Menu for the trayIcon
	QAction *minimizeAction;
	QAction *maximizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	QMenu *trayIconMenu;
	//const QString& notificationId;
	const QString& mPrefix = "MainWindow";
};

#endif // MAINWINDOW_H
