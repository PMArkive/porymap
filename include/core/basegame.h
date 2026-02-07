#pragma once
#ifndef BASEGAMEVERSION_H
#define BASEGAMEVERSION_H

#include <QString>
#include <QIcon>

namespace BaseGame {
    enum Version {
        none, // TODO: Go through and make sure this is a valid state
        pokeruby,
        pokefirered,
        pokeemerald,
    };
    Version stringToVersion(const QString &string);
    QString versionToString(Version version);

    QString getPlayerIconPath(Version version, int character);
    QIcon getPlayerIcon(Version version, int character);
};

#endif // BASEGAMEVERSION_H
