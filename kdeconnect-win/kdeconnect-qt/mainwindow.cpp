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

    ui->plainTextEditDebug->setHidden(true);
    ui->radioButtonLog->setChecked(false);

    KdeConnectConfig* config = new KdeConnectConfig();

	// Signal/Slots connections
    connect(config, &KdeConnectConfig::logMe, this, &MainWindow::displayDebugMessage);
    //connect(config, SIGNAL(logMe()), this, SLOT(displayDebugMessage()));

	QCA::Initializer mQcaInitializer;
	QCA::scanForPlugins();

//	ui->plainTextEditDebug->appendPlainText("QCA Diagnostic Text:\n"); 
//	ui->plainTextEditDebug->appendPlainText(QCA::pluginDiagnosticText());
//	ui->plainTextEditDebug->appendPlainText("QCA Supported Features:\n"); 
//	ui->plainTextEditDebug->appendPlainText(QCA::supportedFeatures().join(","));
	
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
    if(checked)
        ui->plainTextEditDebug->setHidden(false);
    else
        ui->plainTextEditDebug->setHidden(true);
}

void MainWindow::on_pushButtonMyName_clicked()
{
	qCDebug(kcCore) << "pushButtonMyName clicked.";
	displayDebugMessage(QtMsgType::QtDebugMsg, "pushButtonMyName clicked.");
}

void MainWindow::on_pushButtonUnPair_clicked()
{

}

void MainWindow::on_pushButtonRefresh_clicked()
{
	qCDebug(kcCore) << "pushButtonRefresh clicked.";
	displayDebugMessage(QtMsgType::QtDebugMsg, "pushButtonRefresh clicked.");
}

void MainWindow::on_lineEditMyName_textChanged(const QString &arg1)
{

}

