#ifndef KCLOGGER_H
#define KCLOGGER_H

#include "core/coreLibExport.h"

#include <QObject>

class CORELIB_EXPORT KcLogger
		: public QObject
{
	Q_OBJECT

public:
	KcLogger(QObject *parent);
	~KcLogger();
	static KcLogger* instance();

public slots:
	void write(QtMsgType type, const QString &prefix, const QString &msg);

signals:
	void logMe(QtMsgType type, const QString &prefix, const QString &msg);
};

#endif // KCLOGGER_H
