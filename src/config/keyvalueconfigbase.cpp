#include "keyvalueconfigbase.h"
#include "log.h"

#include <QDir>

void KeyValueConfigBase::setRoot(const QString& root) {
    m_root = root;
    QDir dir(m_root);
    if (!m_root.isEmpty() && !dir.exists()) {
        dir.mkpath(m_root);
    }
    // Caching the filepath constructed from m_root + m_filename
    m_filepath = dir.absoluteFilePath(m_filename);
}

bool KeyValueConfigBase::load() {
    QFile file(filepath());
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            logError(QString("Failed to read config file '%1': %2").arg(filepath()).arg(file.errorString()));
            return false;
        }
        if (file.size() == 0) {
            logWarn(QString("Config file '%1' was empty.").arg(filepath()));
            // An empty file isn't a valid JSON file, but other than the warning
            // we'll treat it the same as if it were a JSON file with an empty object.
            initializeFromEmpty();
            return true;
        }
        QJsonParseError parseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            logError(QString("Failed to read config file '%1': %2").arg(filepath()).arg(parseError.errorString()));
            return false;
        }
        if (!jsonDoc.isObject()) {
            logError(QString("Failed to read config file '%1': Expected top level JSON object.").arg(filepath()));
            return false;
        }
        loadFromJson(jsonDoc.object());
        logInfo(QString("Loaded config file '%1'").arg(filename()));
    } else if (!loadLegacy()) {
        // No config file is present (either in the new or old format)
        initializeFromEmpty();
    }
    return true;
}

void KeyValueConfigBase::loadFromJson(const QJsonObject& obj) {
    for (auto it = obj.begin(); it != obj.end(); it++) {
        if (!parseJsonKeyValue(it.key(), it.value())) {
            logWarn(QString("Discarding unrecognized config key '%1'").arg(it.key()));
        }
    }
}

bool KeyValueConfigBase::parseJsonKeyValue(const QString& key, const QJsonValue& value) {
    auto fieldManager = getFieldManager();
    if (!fieldManager || !fieldManager->hasField(key)) return false;

    // Recognized 'key' as a registered field. Let the FieldManager try to assign the value.
    const QStringList errors = fieldManager->setField(key, value);
    if (errors.length() == 1)     logWarn(QString("Failed to read config key '%1': %2").arg(key).arg(errors.at(0)));
    else if (errors.length() > 1) logWarn(QString("Failed to read config key '%1':\n%2").arg(key).arg(errors.join("\n")));
    return true;
}

QJsonObject KeyValueConfigBase::toJson() {
    auto fieldManager = getFieldManager();
    return fieldManager ? fieldManager->getFields() : QJsonObject();
}

bool KeyValueConfigBase::save() {
    QFile file(filepath());
    if (!file.open(QIODevice::WriteOnly)) {
        logError(QString("Could not open config file '%1' for writing: ").arg(filepath()) + file.errorString());
        return false;
    }
   
    QJsonObject savedObject;
    if (m_saveAllFields) {
        savedObject = toJson();
    } else {
        // We limit the output to fields that have changed from the default value.
        // This has a few notable benefits:
        // - It allows changes to the default values to be downstreamed from Porymap.
        // - It reduces diff noise for configs as Porymap's settings change over time.
        // - It discourages manual editing of the file; all settings should be edited in the GUI.
        // If the child class does not reimplement getDefaultJson it returns an empty QJsonObject,
        // and so the default behavior is to output all fields.
        const QJsonObject curObject = toJson();
        const QJsonObject defaultObject = getDefaultJson();
        for (auto it = curObject.begin(); it != curObject.end(); it++) {
            if (it.value() != defaultObject.value(it.key())) {
                savedObject[it.key()] = it.value();
            }
        }
    }

    QJsonDocument doc(savedObject);
    file.write(doc.toJson());
    return true;
}
