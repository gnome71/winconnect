#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kdeconnect-version.h"
#include "core/kdeconnectconfig.h"
#include "core/kclogger.h"
#include "core/daemon.h"
#include "core/networkpackage.h"
#include "core/udplistener.h"

#include <QtCrypto>
#include <QLoggingCategory>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <cassert>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// Start daemon
	Daemon* daemon = new Daemon(this, false);
	// Access to configuration
	KdeConnectConfig* config = new KdeConnectConfig();

	// Setup GUI
	ui->lineEditMyName->setText(config->name());
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

	// Signal/Slots connections
	connect(daemon, SIGNAL(logMe(QtMsgType,QString,QString)), this, SLOT(displayDebugMessage(QtMsgType,QString,QString)));
	connect(config, &KdeConnectConfig::logMe, this, &MainWindow::displayDebugMessage);
	connect(ui->lineEditMyName, &QLineEdit::textEdited, this, &MainWindow::on_lineEditMyName_textChanged);
}

void MainWindow::showDeviceIdentity(const QString &device)
{
	displayDebugMessage(QtMsgType::QtInfoMsg, "UdpSocket ", device);
}

void MainWindow::displayError(int socketError, const QString &message) {
	switch(socketError) {
	case QAbstractSocket::HostNotFoundError:
		displayDebugMessage(QtMsgType::QtWarningMsg, "UdpSocket ", "Host not found.");
		break;
	case QAbstractSocket::ConnectionRefusedError:
		displayDebugMessage(QtMsgType::QtWarningMsg, "UdpSocket ", "Connection refused.");
		break;
	default:
		displayDebugMessage(QtMsgType::QtWarningMsg, "UdpSocket ", message);
	}
}

void MainWindow::displayStatus(QString status) {
	displayDebugMessage(QtMsgType::QtInfoMsg, "UdpSocket ", status);
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

MainWindow::~MainWindow()
{
	delete ui;
}

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

void MainWindow::on_pushButtonMyName_clicked()
{
	ui->pushButtonMyName->setEnabled(false);
}

void MainWindow::on_pushButtonUnPair_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", "pushButtonUnPair clicked.");
}

void MainWindow::on_pushButtonRefresh_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", "pushButtonRefresh clicked.");
}

/**
 * @brief MainWindow::on_lineEditMyName_textChanged
 *
 */
void MainWindow::on_lineEditMyName_textChanged()
{
	if(ui->lineEditMyName->isModified()) {
		ui->pushButtonMyName->setEnabled(true);
		config->setName(ui->lineEditMyName->text());
	}
}

void MainWindow::on_pushButtonQcaInfo_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", config->getQcaInfo());
}

void MainWindow::on_pushButtonSettingInfo_clicked()
{

	QString versionString = QString("KdeConnect-Win Version: %1").arg(KDECONNECT_VERSION_STRING);
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", versionString);
	QDir bcd = config->baseConfigDir();
	QDir dcd = config->deviceConfigDir("1234");
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "BaseConfigDir: " + bcd.path());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "DeviceConfigDir: " + dcd.path());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyName: " + config->name());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyId: " + QString(config->deviceId()));
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyDeviceType: " + config->deviceType());
}
