#pragma once
#ifndef SHORTCUTSCONFIG_H
#define SHORTCUTSCONFIG_H

#include "keyvalueconfigbase.h"

#include <QStandardPaths>

class QAction;
class Shortcut;

class ShortcutsConfig : public KeyValueConfigBase
{
public:
    ShortcutsConfig() : KeyValueConfigBase(QStringLiteral("shortcuts.json")) {
        setRoot(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    virtual QJsonObject toJson() override;
    virtual void loadFromJson(const QJsonObject& obj) override;

    // Call this before applying user shortcuts so that the user can restore defaults.
    void setDefaultShortcuts(const QObjectList& objects);
    QList<QKeySequence> defaultShortcuts(const QObject *object) const;

    void setUserShortcuts(const QObjectList& objects);
    void setUserShortcuts(const QMultiMap<const QObject *, QKeySequence>& objects_keySequences);
    QList<QKeySequence> userShortcuts(const QObject *object) const;

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;

private:
    QMultiMap<QString, QKeySequence> user_shortcuts;
    QMultiMap<QString, QKeySequence> default_shortcuts;

    enum StoreType {
        User,
        Default
    };

    QString cfgKey(const QObject *object) const;
    QList<QKeySequence> currentShortcuts(const QObject *object) const;

    void storeShortcutsFromList(StoreType storeType, const QObjectList& objects);
    void storeShortcuts(
            StoreType storeType,
            const QString& cfgKey,
            const QList<QKeySequence>& keySequences);
};

extern ShortcutsConfig shortcutsConfig;

#endif // SHORTCUTSCONFIG_H
