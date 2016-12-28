#include "kclogger.h"

Q_GLOBAL_STATIC(KcLogger*, s_instance)

KcLogger* KcLogger::instance()
{
	Q_ASSERT(s_instance.exists());
	return *s_instance;
}

KcLogger::KcLogger(QObject *parent)
	: QObject(parent)
{
	Q_ASSERT(!s_instance.exists());
	*s_instance = this;
}

void KcLogger::write(QtMsgType type, const QString &prefix, const QString &msg)
{
	emit logMe(type, prefix, msg);
}

KcLogger::~KcLogger()
{
}
