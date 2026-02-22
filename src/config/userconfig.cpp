#include "userconfig.h"

// TODO: This should eventually be contained by each individual Project instance.
UserConfig userConfig;

void UserConfig::loadFromJson(const QJsonObject& obj) {
    KeyValueConfigBase::loadFromJson(obj);

    // Enforce this setting for userConfig's custom scripts
    for (auto& settings : this->customScripts) settings.userOnly = true;
}

QJsonObject UserConfig::getDefaultJson() const {
    UserConfig defaultConfig;
    return defaultConfig.toJson();
}
