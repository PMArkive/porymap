#pragma once
#ifndef UTILITY_H
#define UTILITY_H

#include <QString>
#include <QLineEdit>
#include <QColorSpace>

enum NumberSystemMode {
    Both,
    Decimal,
    Hexadecimal,
};

namespace Util {
    void numericalModeSort(QStringList &list);
    int roundUpToMultiple(int numToRound, int multiple);
    QString toDefineCase(QString input);
    QString toHexString(uint32_t value, int minLength = 0);
    QString toHtmlParagraph(const QString &text);
    QString stripPrefix(const QString &s, const QString &prefix);
    Qt::Orientations getOrientation(bool xflip, bool yflip);
    QString replaceExtension(const QString &path, const QString &newExtension);
    void setErrorStylesheet(QLineEdit *lineEdit, bool isError);
    QString toStylesheetString(const QFont &font);
    void show(QWidget *w);
    QString mkpath(const QString& dirPath);
    QString getFileHash(const QString &filepath);

    template <typename T>
    QString getNumberSystemString(T value, NumberSystemMode mode, int minHexLength = 0) {
        switch (mode) {
        case NumberSystemMode::Hexadecimal: return toHexString(value, minHexLength);
        case NumberSystemMode::Decimal: return QString::number(value);
        case NumberSystemMode::Both: return QString("%1 (%2)")
                                            .arg(getNumberSystemString(value, NumberSystemMode::Hexadecimal, minHexLength))
                                            .arg(getNumberSystemString(value, NumberSystemMode::Decimal));
        }
        Q_ASSERT("Cannot get string for invalid NumberSystemMode.");
        return QString();
    }

    // Given a QMap<T,QString>, erases all entries with empty strings.
    // Returns the number of entries erased.
    template <typename T>
    int removeEmptyStrings(QMap<T,QString> *map) {
        if (!map) return 0;
        int numRemoved = 0;
        for (auto it = map->begin(); it != map->end();) {
            if (it.value().isEmpty()) it = map->erase(it);
            else {it++; numRemoved++;}
        }
        return numRemoved;
    }
}

#endif // UTILITY_H
