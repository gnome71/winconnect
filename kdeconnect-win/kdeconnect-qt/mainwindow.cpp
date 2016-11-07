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

	// Signal/Slots connections
	connect(ui->lineEditMyName, &QLineEdit::textEdited, this, &MainWindow::on_lineEditMyName_textChanged);
	connect(ui->listViewDevice, &QListView::activated, this, &MainWindow::on_listViewDevice_clicked);
	connect(m_dmodel, &DevicesModel::dataChanged, this, &MainWindow::on_dataChanged);
	connect(m_logger, &KcLogger::logMe, this, &MainWindow::displayDebugMessage);
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
	m_logger->disconnect();
	m_daemon->releaseDiscoveryMode(createId());
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

void MainWindow::on_pushButtonQcaInfo_clicked()
{
	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", KdeConnectConfig::instance()->getQcaInfo());
//	m_currentIndex = ui->listViewDevice->selectionModel()->selectedRows().at(0);
//	m_currentDevice = m_daemon->getDevice(m_dmodel->data(m_currentIndex, DevicesModel::NameModelRole).toString());
//	displayDebugMessage(QtMsgType::QtDebugMsg, "MainWindow", m_currentDevice->encryptionInfo());
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

void MainWindow::on_listViewDevice_clicked(QModelIndex index)
{
	m_currentDevice = m_daemon->getDevice(m_dmodel->data(index, DevicesModel::NameModelRole).toString());

	ui->labelPairedDevice->setText(m_dmodel->data(index, Qt::DisplayRole).toString());
	// Reachable 0x01, Paired 0x02
	if(m_dmodel->data(index, DevicesModel::StatusModelRole) == 1) {
		ui->labelPairedDevice->setEnabled(true);
		ui->pushButtonUnPair->setText("Pair");
		ui->pushButtonUnPair->setEnabled(true);
	}
//	else if(m_dmodel->data(index, DevicesModel::StatusModelRole) == 2) {
//		ui->labelPairedDevice->setEnabled(true);
//		ui->pushButtonUnPair->setText("Unpair");
//		ui->pushButtonUnPair->setEnabled(true);
//	}
	else if(m_dmodel->data(index, DevicesModel::StatusModelRole) == 3) {
		ui->labelPairedDevice->setEnabled(true);
		ui->pushButtonUnPair->setText("Unpair");
		ui->pushButtonUnPair->setEnabled(true);
	}
}

void MainWindow::on_dataChanged(QModelIndex tl, QModelIndex br)
{
	Q_UNUSED(br);
	QString msg = "Data changed for: " + m_dmodel->data(tl, Qt::DisplayRole).toString()
			+ " " + m_dmodel->data(tl, Qt::UserRole).toString();
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, mPrefix, msg);
	//qDebug() << m_daemon->devices();

	on_listViewDevice_clicked(tl);
}
