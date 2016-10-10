#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/kdeconnectconfig.h"
#include "core/kclogger.h"

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

	config = new KdeConnectConfig();

	// Signal/Slots connections
    QObject::connect(config, &KdeConnectConfig::logMe, this, &MainWindow::displayDebugMessage);


}

void MainWindow::displayDebugMessage(QtMsgType type, const QString &msg)
{
	bool do_abort = false;
	const char* msgTypeStr = NULL;
	switch (type) {
	case QtDebugMsg:
		msgTypeStr = "Debug";
		break;
	case QtWarningMsg:
		msgTypeStr = "Warning";
		break;
	case QtCriticalMsg:
		msgTypeStr = "Critical";
		break;
	case QtFatalMsg:
		msgTypeStr = "Fatal";
		do_abort = true;
	default:
		assert(0);
		return;
	}
	QTime now = QTime::currentTime();
	QString formattedMessage =
		QString::fromLatin1("%1 %2 %3")
		.arg(now.toString("hh:mm:ss:zzz"))
		.arg(msgTypeStr).arg(msg);
	// print on console:
	fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
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
	displayDebugMessage(QtMsgType::QtDebugMsg, "pushButtonMyName clicked.");
}

void MainWindow::on_pushButtonUnPair_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "pushButtonUnPair clicked.");
}

void MainWindow::on_pushButtonRefresh_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "pushButtonRefresh clicked");
}

void MainWindow::on_lineEditMyName_textChanged(const QString &arg1)
{

}

void MainWindow::on_pushButtonQcaInfo_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, config->getQcaInfo());
}

void MainWindow::on_pushButtonSettingInfo_clicked()
{
	QDir bcd = config->baseConfigDir();
	QDir dcd = config->deviceConfigDir("1234");
	displayDebugMessage(QtMsgType::QtDebugMsg, "BaseConfigDir:    " + bcd.path());
	displayDebugMessage(QtMsgType::QtDebugMsg, "DeviceConfigDir: " + dcd.path());
}
