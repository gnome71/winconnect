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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "winconnect-version.h"
#include "core/kdeconnectconfig.h"
#include "core/kclogger.h"
#include "core/daemon.h"
#include "core/networkpackage.h"

#include <QtCrypto>
#include <QLoggingCategory>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QMenu>
#include <QLibraryInfo>
#include <cassert>

static QString createId() { return QStringLiteral("kcw")+QString::number(QCoreApplication::applicationPid()); }

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	QCA::Initializer mQcaInitializer;

	ui->setupUi(this);

	// create KcLogger instance
	m_logger = new KcLogger(this);

	// Access to configuration
	m_config = KdeConnectConfig::instance();

	// Setup GUI
	ui->lineEditMyName->setText(m_config->name());
#ifdef QT_DEBUG
	ui->plainTextEditDebug->setHidden(false);
	ui->radioButtonLog->setChecked(true);
	ui->pushButtonQcaInfo->setHidden(false);
	ui->pushButtonSettingInfo->setHidden(false);
#else
	ui->plainTextEditDebug->setHidden(true);
	ui->radioButtonLog->setChecked(false);
	ui->pushButtonQcaInfo->setHidden(true);
	ui->pushButtonSettingInfo->setHidden(true);
#endif

	// Start daemon
	m_daemon = new Daemon(this, false);

	// Set model/view
	m_dmodel = new DevicesModel();
	ui->listViewDevice->setModel(m_dmodel);

	// Create tray stuff
	createTrayActions();
	createTrayIcon();

	// Signal/Slots connections
	connect(ui->lineEditMyName, &QLineEdit::textEdited, this, &MainWindow::on_lineEditMyName_textChanged);
	connect(ui->listViewDevice, &QListView::activated, this, &MainWindow::on_listViewDevice_clicked);
	connect(m_dmodel, &DevicesModel::dataChanged, this, &MainWindow::on_dataChanged);
	connect(m_daemon, &Daemon::askPairing, this, &MainWindow::showMessage);
	connect(m_logger, &KcLogger::logMe, this, &MainWindow::displayDebugMessage);
	connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::messageClicked);
}

/**
 * @brief MainWindow::displayDebugMessage
 * @param type
 * @param msg
 */
void MainWindow::displayDebugMessage(QtMsgType type, const QString &prefix, const QString &msg)
{
	const char* msgTypeStr = "";
	switch (type) {
	case QtInfoMsg:
		msgTypeStr = "[I]";
		break;
	case QtDebugMsg:
		msgTypeStr = "[D]";
		break;
	case QtWarningMsg:
		msgTypeStr = "[W]";
		break;
	case QtCriticalMsg:
		msgTypeStr = "[C]";
		break;
	case QtFatalMsg:
		msgTypeStr = "[E]";
		break;
	}
	QTime now = QTime::currentTime();
	QString formattedMessage =
			QString::fromLatin1("%1 %2 %3 %4")
			.arg(now.toString("hh:mm:ss:zzz"))
			.arg(prefix).arg(msgTypeStr).arg(msg);
	// print on console:
	//fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
	// print in debug log window
	{
		bool isMainThread = QThread::currentThread() == QApplication::instance()->thread();
		if (ui->plainTextEditDebug != 0)
		{
			if (isMainThread)
				ui->plainTextEditDebug->appendPlainText(formattedMessage);
			else // additional code, so that qDebug calls in threads will work aswell
				QMetaObject::invokeMethod(ui->plainTextEditDebug, "appendPlainText", Qt::QueuedConnection, Q_ARG(QString, formattedMessage));
		}
	}
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
	m_logger->disconnect();
	m_daemon->releaseDiscoveryMode(createId());
	delete ui;
}

/**
 * @brief MainWindow::on_radioButtonLog_toggled
 * @param checked
 */
void MainWindow::on_radioButtonLog_toggled(bool checked)
{
	if(checked) {
		ui->plainTextEditDebug->setHidden(false);
		ui->pushButtonQcaInfo->setHidden(false);
		ui->pushButtonSettingInfo->setHidden(false);
	}
	else {
		ui->plainTextEditDebug->setHidden(true);
		ui->pushButtonQcaInfo->setHidden(true);
		ui->pushButtonSettingInfo->setHidden(true);
	}
}

/**
 * @brief MainWindow::on_pushButtonMyName_clicked
 */
void MainWindow::on_pushButtonMyName_clicked()
{
	ui->pushButtonMyName->setEnabled(false);
}

/**
 * @brief MainWindow::on_pushButtonUnPair_clicked
 */
void MainWindow::on_pushButtonUnPair_clicked()
{
	if(!ui->listViewDevice->selectionModel()->hasSelection())
		return;
	m_currentIndex = ui->listViewDevice->selectionModel()->selectedRows().at(0);
	QString id = m_dmodel->data(m_currentIndex, DevicesModel::IdModelRole).toString();
	m_currentDevice = m_daemon->getDevice(m_dmodel->data(m_currentIndex, DevicesModel::IdModelRole).toString());
	if(m_currentDevice == nullptr)
		return;
	QString msg = "Current device: " + m_currentDevice->name();
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", msg);

	if(m_currentDevice->isReachable() && m_currentDevice->isTrusted()) {
		m_currentDevice->unpair();
		return;
	}
	if(m_currentDevice->isReachable() && !m_currentDevice->isTrusted()) {
		m_currentDevice->requestPair();
		return;
	}
}

/**
 * @brief MainWindow::on_pushButtonRefresh_clicked
 */
void MainWindow::on_pushButtonRefresh_clicked()
{
	m_daemon->acquireDiscoveryMode(createId());
	m_daemon->forceOnNetworkChange();
}

/**
 * @brief MainWindow::on_lineEditMyName_textChanged
 *
 */
void MainWindow::on_lineEditMyName_textChanged()
{
	if(ui->lineEditMyName->isModified()) {
		ui->pushButtonMyName->setEnabled(true);
		KdeConnectConfig::instance()->setName(ui->lineEditMyName->text());
	}
}

/**
 * @brief MainWindow::on_pushButtonQcaInfo_clicked
 */
void MainWindow::on_pushButtonQcaInfo_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", KdeConnectConfig::instance()->getQcaInfo());
	if(!ui->listViewDevice->selectionModel()->hasSelection())
		return;
	m_currentIndex = ui->listViewDevice->selectionModel()->selectedRows().at(0);
	m_currentDevice = m_daemon->getDevice(m_dmodel->data(m_currentIndex, DevicesModel::IdModelRole).toString());
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", m_currentDevice->encryptionInfo());
}

/**
 * @brief MainWindow::on_pushButtonSettingInfo_clicked
 */
void MainWindow::on_pushButtonSettingInfo_clicked()
{
	QString versionString = QString("KdeConnect-Win Version: %1").arg(KDECONNECT_VERSION_STRING);
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", versionString);
	QDir bcd = KdeConnectConfig::instance()->baseConfigDir();
	QDir dcd = KdeConnectConfig::instance()->deviceConfigDir("1234");
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "BaseConfigDir: " + bcd.path());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "DeviceConfigDir: " + dcd.path());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "PluginDir: " + QLibraryInfo::location(QLibraryInfo::PluginsPath));
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyName: " + KdeConnectConfig::instance()->name());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyId: " + QString(KdeConnectConfig::instance()->deviceId()));
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyDeviceType: " + KdeConnectConfig::instance()->deviceType());
}

/**
 * @brief MainWindow::on_listViewDevice_clicked
 * @param index
 */
void MainWindow::on_listViewDevice_clicked(QModelIndex index)
{
	//m_currentDevice = m_daemon->getDevice(m_dmodel->data(index, DevicesModel::NameModelRole).toString());

	ui->labelPairedDevice->setText(m_dmodel->data(index, Qt::DisplayRole).toString());
	// Paired 0x01, Reachable 0x02, combineable
	if(m_dmodel->data(index, DevicesModel::StatusModelRole) == 1) {
		ui->labelPairedDevice->setEnabled(true);
		ui->pushButtonUnPair->setText("Pair");
		ui->pushButtonUnPair->setEnabled(true);
	}
	else if(m_dmodel->data(index, DevicesModel::StatusModelRole) == 2) {
		ui->labelPairedDevice->setEnabled(true);
		ui->pushButtonUnPair->setText("Pair");
		ui->pushButtonUnPair->setEnabled(true);
	}
	else if(m_dmodel->data(index, DevicesModel::StatusModelRole) == 3) {
		ui->labelPairedDevice->setEnabled(true);
		ui->pushButtonUnPair->setText("Unpair");
		ui->pushButtonUnPair->setEnabled(true);
	}

	QString tt = "";
	Q_FOREACH(const QString& d, m_daemon->devices()) {
		tt.append(m_daemon->getDevice(d)->name() + "\n");
	}
	trayIcon->setToolTip(tt);
}

/**
 * @brief MainWindow::on_dataChanged
 * @param tl, index of the changed device in the model
 * @param br, not used
 */
void MainWindow::on_dataChanged(QModelIndex tl, QModelIndex br)
{
	Q_UNUSED(br);
	QString msg = "Data changed for: " + m_dmodel->data(tl, Qt::DisplayRole).toString()
			+ ", Id: " + m_dmodel->data(tl, Qt::UserRole).toString();
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, mPrefix, msg);
	//qDebug() << m_daemon->devices();

	on_listViewDevice_clicked(tl);
}

/**
 * @brief MainWindow::on_pushButtonOk_clicked
 */
void MainWindow::on_pushButtonOk_clicked()
{
	if (trayIcon->isVisible()) {
		this->hide();
	}
}

/**
 * @brief MainWindow::createTrayIcon
 */
void MainWindow::createTrayIcon()
{
	qDebug() << "MainWindow Tray:" << m_daemon->devices();
	trayIcon = new QSystemTrayIcon(QIcon(":/icons/AppIcon.png"), this);
	trayIcon->setToolTip("WinConnect");
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(minimizeAction);
	trayIconMenu->addAction(maximizeAction);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->show();
}

/**
 * @brief MainWindow::createTrayActions
 */
void MainWindow::createTrayActions()
{
	minimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

	maximizeAction = new QAction(tr("Ma&ximize"), this);
	connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

/**
 * @brief MainWindow::showMessage
 * @param dev
 * @param msg
 */
void MainWindow::showMessage(const QString& dev, const QString &msg)
{
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, "  NOTIFY  ", dev + ": " + msg);
	trayIcon->showMessage(dev, msg, QSystemTrayIcon::Information, 5000);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
	case QSystemTrayIcon::DoubleClick:
		this->showNormal();
	default:
		;
	}
}

/**
 * @brief MainWindow::messageClicked
 */
void MainWindow::messageClicked()
{
	qDebug() << QObject::sender();

}

