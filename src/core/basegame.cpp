#include "basegame.h"

#include <QList>
#include <QMap>

// If a string exclusively contains one version name we assume its identity,
// otherwise we leave it unknown and we'll need the user to tell us the version.
BaseGame::Version BaseGame::stringToVersion(const QString &input_) {
    static const QMap<Version, QStringList> versionDetectNames = {
        {Version::pokeruby, {"ruby", "sapphire"}},
        {Version::pokefirered, {"firered", "leafgreen"}},
        {Version::pokeemerald, {"emerald"}},
    };

    const QString input(input_.toLower());
    BaseGame::Version version = BaseGame::Version::none;
    for (auto it = versionDetectNames.begin(); it != versionDetectNames.end(); it++) {
        // Compare the given string to all the possible names for this game version
        for (const auto &name : it.value()) {
            if (input.contains(name)) {
                if (version != BaseGame::Version::none) {
                    // The given string matches multiple versions, so we can't be sure which it is.
                    return BaseGame::Version::none;
                }
                version = it.key();
                break;
            }
        }
    }
    // We finished checking the names for each version; the name either matched 1 version or none.
    return version;
}

// TODO: Make sure empty string is ok everywhere this is used
QString BaseGame::versionToString(BaseGame::Version version) {
    static const QMap<Version, QString> map = {
        {Version::pokeruby,    "pokeruby"},
        {Version::pokefirered, "pokefirered"},
        {Version::pokeemerald, "pokeemerald"},
    };
    return map.value(version);
}

QString BaseGame::getPlayerIconPath(BaseGame::Version version, int character) {
    if (version == BaseGame::Version::pokeemerald) {
        static const QStringList paths = { QStringLiteral(":/icons/player/brendan_em.ico"),
                                           QStringLiteral(":/icons/player/may_em.ico"), };
        return paths.value(character);
    } else if (version == BaseGame::Version::pokefirered) {
        static const QStringList paths = { QStringLiteral(":/icons/player/red.ico"),
                                           QStringLiteral(":/icons/player/green.ico"), };
        return paths.value(character);
    } else if (version == BaseGame::Version::pokeruby) {
        static const QStringList paths = { QStringLiteral(":/icons/player/brendan_rs.ico"),
                                           QStringLiteral(":/icons/player/may_rs.ico"), };
        return paths.value(character);
    }
    return QString();
}

QIcon BaseGame::getPlayerIcon(BaseGame::Version baseGameVersion, int character) {
    return QIcon(getPlayerIconPath(baseGameVersion, character));
}
