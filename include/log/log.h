#pragma once
#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QHash>

enum class LogType {
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
};

inline uint qHash(const LogType &key, uint seed = 0) {
    return qHash(static_cast<int>(key), seed);
}

void logInit();
void logInfo(const QString &message);
void logWarn(const QString &message);
void logError(const QString &message);

QString getLogPath();
QString getMostRecentError();

#endif // LOG_H
