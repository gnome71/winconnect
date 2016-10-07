#include "logger.h"

#include <QDebug>

Logger::Logger(QObject *parent) : QObject(parent)
{

}

void Logger::debug(QString msg)
{
    qDebug() << "D:" << parent()->objectName() << ": " << msg;
}
