#pragma once
#ifndef KEYVALUECONFIGBASE_H
#define KEYVALUECONFIGBASE_H

#include <QJsonObject>
#include <QString>

#include "fieldmanager.h"

class KeyValueConfigBase
{
public:
    explicit KeyValueConfigBase(const QString& filename)
        : m_root(QString()),
          m_filename(filename),
          m_filepath(filename)
    { };
    virtual ~KeyValueConfigBase() {};

    virtual bool save();
    virtual bool load();

    virtual QJsonObject toJson();
    virtual void loadFromJson(const QJsonObject& obj);

    void setRoot(const QString& dir);
    QString root() const { return m_root; }
    QString filepath() const { return m_filepath; }
    QString filename() const { return m_filename; }
protected:
    virtual void initializeFromEmpty() {};
    virtual QJsonObject getDefaultJson() const { return QJsonObject(); }

    virtual FieldManager* getFieldManager() { return nullptr; }

    virtual bool parseJsonKeyValue(const QString& key, const QJsonValue& value);
    virtual bool parseLegacyKeyValue(const QString& , const QString& ) {return false;}

    QString m_root;
    QString m_filename;
    QString m_filepath;
private:
    bool loadLegacy();

    bool m_saveAllFields = false;
};

#endif // KEYVALUECONFIGBASE_H
