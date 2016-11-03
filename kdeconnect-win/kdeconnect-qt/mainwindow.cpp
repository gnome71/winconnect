#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kdeconnect-version.h"
#include "core/kdeconnectconfig.h"
#include "core/kclogger.h"
#include "core/daemon.h"
#include "core/networkpackage.h"

#include <QtCrypto>
#include <QLoggingCategory>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <cassert>

static QString createId() { return QStringLiteral("kcw")+QString::number(QCoreApplication::applicationPid()); }

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	QCA::Initializer mQcaInitializer;

	ui->setupUi(this);

	// create KcLogger instance
	KcLogger* logger = new KcLogger(this);

	// Access to configuration
	KdeConnectConfig* config = KdeConnectConfig::instance();

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

	// Start daemon
	daemon = new Daemon(this, false);

	// Signal/Slots connections
	connect(ui->lineEditMyName, &QLineEdit::textEdited, this, &MainWindow::on_lineEditMyName_textChanged);
	connect(logger, &KcLogger::logMe, this, &MainWindow::displayDebugMessage);
	//connect(daemon, &Daemon::deviceAdded)
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
	daemon->acquireDiscoveryMode(createId());
	daemon->forceOnNetworkChange();
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

void MainWindow::on_pushButtonQcaInfo_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", KdeConnectConfig::instance()->getQcaInfo());
}

void MainWindow::on_pushButtonSettingInfo_clicked()
{

	QString versionString = QString("KdeConnect-Win Version: %1").arg(KDECONNECT_VERSION_STRING);
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", versionString);
	QDir bcd = KdeConnectConfig::instance()->baseConfigDir();
	QDir dcd = KdeConnectConfig::instance()->deviceConfigDir("1234");
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "BaseConfigDir: " + bcd.path());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "DeviceConfigDir: " + dcd.path());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyName: " + KdeConnectConfig::instance()->name());
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyId: " + QString(KdeConnectConfig::instance()->deviceId()));
	displayDebugMessage(QtMsgType::QtInfoMsg, "MainWindow", "MyDeviceType: " + KdeConnectConfig::instance()->deviceType());
}
