#pragma once
#ifndef USERCONFIG_H
#define USERCONFIG_H

#include "keyvalueconfigbase.h"

class UserConfig: public KeyValueConfigBase
{
public:
    UserConfig(const QString& root = QString()) : KeyValueConfigBase(QStringLiteral("porymap.user.json")) {
        setRoot(root);
    }

    virtual void loadFromJson(const QJsonObject& obj) override;

    QString projectDir() const { return m_root; } // Alias for root()

    QString recentMapOrLayout;
    QString prefabsFilepath;
    bool prefabsImportPrompted = false;
    bool useEncounterJson = true;
    QList<PluginSettings> plugins;

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;

    FieldManager* getFieldManager() override {
        if (!m_fm) {
            m_fm = std::make_shared<FieldManager>();
            m_fm->addField(&this->recentMapOrLayout, "recent_map_or_layout");
            m_fm->addField(&this->prefabsFilepath, "prefabs_filepath");
            m_fm->addField(&this->prefabsImportPrompted, "prefabs_import_prompted");
            m_fm->addField(&this->useEncounterJson, "use_encounter_json");
            m_fm->addField(&this->plugins, "plugins");
        }
        return m_fm.get();
    }

private:
    std::shared_ptr<FieldManager> m_fm = nullptr;
};

extern UserConfig userConfig;

#endif // USERCONFIG_H
