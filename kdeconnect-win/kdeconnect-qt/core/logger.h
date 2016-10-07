#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);

    void debug(QString msg);
signals:

public slots:
};

#endif // LOGGER_H
