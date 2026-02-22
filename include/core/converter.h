#pragma once
#ifndef CONVERTER_H
#define CONVERTER_H

#include <QJsonValue>
#include <QVersionNumber>

#include "magic_enum.hpp"
#include "orderedset.h"
#include "scriptsettings.h"
#include "gridsettings.h"
#include "basegame.h"

/*
    These are templates for type conversion to/from JSON,
    though other type conversions can be implemented here too.

    This is mostly useful when converting the type is complicated,
    or when the type is generalized away.


    ## Example Usage ##
    QSize size;
    QJsonValue json = Converter<QSize>::toJson(size);
    QSize sameSize = Converter<QSize>::fromJson(json);


    ## Adding a new conversion ##
    To add a new type conversion, add a new 'Converter' template:

    template <>
    struct Converter<NewType> : DefaultConverter<NewType> {
        // And re-implement any of the desired conversion functions.
        // Any functions not implemented will be inherited from DefaultConverter.
        static QJsonValue toJson(const NewType& value) {
            // your conversion to JSON
            return QJsonValue();
        }
        static NewType fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
            // your conversion from JSON
            return NewType();
        }
    };

    Note: When serializing to/from JSON, anything that can be serialized to/from
          a string is trivially serializable for JSON. In this case, rather than
          inheriting from 'DefaultConverter' and reimplementing 'toJson' and 'fromJson',
          you can inherit from 'DefaultStringConverter' and/or reimplement 'toString'/'fromString'.
          Appropriately implementing 'toString'/'fromString' has the added benefit that your type
          can automatically be used as a JSON key if it for example appears as the key in a QMap.

    // TODO: Document clamp

*/


template <typename T>
struct DefaultConverter {
    // Defaults to straightforward QJsonValue construction.
    // This handles most of the primitive types.
    static QJsonValue toJson(const T& value) {
        return QJsonValue{value};
    }
    static T fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const QVariant v = json.toVariant();
        if (!v.canConvert<T>()) {
            if (errors) errors->append(QString("Can't convert from type '%1'").arg(v.typeName()));
            // Failed conversion will return a default-constructed object below
        }
        return v.value<T>();
    }

    // Default to identity
    static QString toString(const T& value) {return value;}
    static T fromString(const QString& string, QStringList* = nullptr) {return string;}

    static T clamp(const T& value, const T& min, const T& max, QStringList* errors = nullptr) {
        Q_ASSERT(min <= max);
        if (value < min) {
            if (errors) errors->append("Value too low");
            return min;
        }
        if (value > max) {
            if (errors) errors->append("Value too high");
            return max;
        }
        return value;
    }
};

template <typename T, typename Enable = void>
struct Converter : DefaultConverter<T> {};

// This template implements JSON conversion by first converting the data to/from a string.
// This allows any type that can describe how to stringify itself to automatically also
// support JSON conversion with no additional work.
template <typename T>
struct DefaultStringConverter : DefaultConverter<T> {
    static QJsonValue toJson(const T& value) {
        return Converter<QString>::toJson(Converter<T>::toString(value));
    }
    static T fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto string = Converter<QString>::fromJson(json, errors);
        return Converter<T>::fromString(string, errors);
    }
    // Many types have a 'toString' function, so we default to trying that.
    static QString toString(const T& value) {
        return value.toString();
    }
};

template <>
struct Converter<QUrl> : DefaultStringConverter<QUrl> {};

template <>
struct Converter<QKeySequence> : DefaultStringConverter<QKeySequence> {};

template <>
struct Converter<uint32_t> : DefaultConverter<uint32_t> {
    // Constructing a QJsonValue from uint32_t is ambiguous, so we need an explicit cast.
    static QJsonValue toJson(uint32_t value) {
        return QJsonValue{static_cast<qint64>(value)};
    }
};

// Template for generic enum values.
// Converts JSON -> string/int -> enum, handling the unsafe conversion if the int is out of range of the enum.
// Qt has a system for this (Q_ENUM) but they don't use it for all their internal enums, so we use magic_enum instead.
template <typename T>
struct Converter<T, std::enable_if_t<std::is_enum_v<T>>> : DefaultStringConverter<T> {
    static QString toString(const T& value) {
        const std::string s = std::string(magic_enum::enum_name(value));
        return QString::fromStdString(s);
    }
    static T fromString(const QString& string, QStringList* errors = nullptr) {
        auto e = magic_enum::enum_cast<T>(string.toStdString(), magic_enum::case_insensitive);
        if (!e.has_value()) {
            if (errors) errors->append(QString("'%1' is not a named enum value.").arg(string));
            return magic_enum::enum_value<T>(0);
        }
        return e.value();
    }
    // When reading from JSON, handle either the named enum or an enum's number value.
    static T fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        if (json.isString()) return Converter<T>::fromString(json.toString());
        auto value = Converter<int>::fromJson(json, errors);
        auto e = magic_enum::enum_cast<T>(value);
        if (!e.has_value()) {
            if (errors) errors->append(QString("'%1' is out of range of enum.").arg(QString::number(value)));
            return magic_enum::enum_value<T>(0);
        }
        return e.value();
    }
};

template <>
struct Converter<QVersionNumber> : DefaultStringConverter<QVersionNumber> {
    static QVersionNumber fromString(const QString& string, QStringList* = nullptr) {
        return QVersionNumber::fromString(string);
    }
};

template <>
struct Converter<QDateTime> : DefaultStringConverter<QDateTime> {
    static QString toString(const QDateTime& value) {
        return value.toUTC().toString();
    }
    static QDateTime fromString(const QString& string, QStringList* = nullptr) {
        return QDateTime::fromString(string).toLocalTime();
    }
};

template <>
struct Converter<QColor> : DefaultStringConverter<QColor> {
    static QString toString(const QColor& value) {
        return value.name();
    }
    static QColor fromString(const QString& string, QStringList* errors = nullptr) {
        const QColor color(string);
        if (!color.isValid()) {
            if (errors) errors->append(QString("'%1' is not a valid color.").arg(string));
            return QColorConstants::Black;
        }
        return color;
    }
};

template <>
struct Converter<QFont> : DefaultStringConverter<QFont> {
    static QFont fromString(const QString& string, QStringList* errors = nullptr) {
        QFont font;
        if (!font.fromString(string) && errors) {
           errors->append(QString("'%1' is not a valid font description.").arg(string));
        }
        return font;
    }
};

template <>
struct Converter<BaseGame::Version> : DefaultStringConverter<BaseGame::Version> {
    static QString toString(const BaseGame::Version& value) {
        return BaseGame::versionToString(value);
    }
    static BaseGame::Version fromString(const QString& string, QStringList* = nullptr) {
        return BaseGame::stringToVersion(string);
    }
};

template <typename T>
struct Converter<std::optional<T>> : DefaultConverter<std::optional<T>> {
    static QJsonValue toJson(const std::optional<T>& optional) {
        return optional.has_value() ? Converter<T>::toJson(optional.value()) : QJsonValue();
    }
    static std::optional<T> fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        if (json.isNull()) return {};
        return Converter<T>::fromJson(json, errors);
    }
};

template <typename T>
struct Converter<QList<T>> : DefaultConverter<QList<T>> {
    static QJsonValue toJson(const QList<T>& list) {
        QJsonArray arr;
        for (auto& elem : list) arr.append(Converter<T>::toJson(elem));
        return arr;
    }
    static QList<T> fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto arr = Converter<QJsonArray>::fromJson(json, errors);
        QList<T> list;
        for (auto& elem : arr) list.append(Converter<T>::fromJson(elem, errors));
        return list;
    }
};

template <typename K, typename V>
struct Converter<QMap<K,V>> : DefaultConverter<QMap<K,V>> {
    static QJsonObject toJson(const QMap<K,V>& map) {
        QJsonObject obj;
        for (auto it = map.begin(); it != map.end(); it++) {
            const QString key = Converter<K>::toString(it.key());
            obj[key] = Converter<V>::toJson(it.value());
        }
        return obj;
    }
    static QMap<K,V> fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto obj = Converter<QJsonObject>::fromJson(json, errors);
        QMap<K,V> map;
        for (auto it = obj.begin(); it != obj.end(); it++) {
            const auto key = Converter<K>::fromString(it.key(), errors);
            map.insert(key, Converter<V>::fromJson(it.value(), errors));
        }
        return map;
    }
};

template <typename K, typename V>
struct Converter<QMultiMap<K,V>> : DefaultConverter<QMultiMap<K,V>> {
    static QJsonObject toJson(const QMultiMap<K,V>& map) {
        QJsonObject obj;
        for (const auto& uniqueKey : map.uniqueKeys()) {
            const QString key = Converter<K>::toString(uniqueKey);
            obj[key] = Converter<QList<V>>::toJson(map.values(uniqueKey));
        }
        return obj;
    }
    static QMultiMap<K,V> fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto obj = Converter<QJsonObject>::fromJson(json, errors);
        QMultiMap<K,V> map;
        for (auto it = obj.begin(); it != obj.end(); it++) {
            const auto key = Converter<K>::fromString(it.key(), errors);
            const auto values = Converter<QList<V>>::fromJson(it.value(), errors);
            for (const auto& value : values) map.insert(key, value);
        }
        return map;
    }
};

template <typename T>
struct Converter<OrderedSet<T>> : DefaultConverter<OrderedSet<T>> {
    static QJsonValue toJson(const OrderedSet<T>& set) {
        QJsonArray arr;
        for (auto& elem : set) arr.append(Converter<T>::toJson(elem));
        return arr;
    }
    static OrderedSet<T> fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto arr = Converter<QJsonArray>::fromJson(json, errors);
        OrderedSet<T> set;
        for (auto& elem : arr) set.insert(Converter<T>::fromJson(elem, errors));
        return set;
    }
};

template <>
struct Converter<QSize> : DefaultConverter<QSize> {
    static QJsonValue toJson(const QSize& value) {
        QJsonObject obj;
        obj["width"] = value.width();
        obj["height"] = value.height();
        return obj;
    }
    static QSize fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto obj = Converter<QJsonObject>::fromJson(json, errors);
        QSize size;
        size.setWidth(obj.value("width").toInt());
        size.setHeight(obj.value("height").toInt());
        return size;
    }
    static QSize clamp(const QSize& value, const QSize& min, const QSize& max, const QStringList* = nullptr) {
        Q_ASSERT(min.width() <= max.width());
        Q_ASSERT(min.height() <= max.height());
        QSize size = value;
        if (value.width() < min.width() || value.height() < min.height()) size = value.expandedTo(min);
        if (value.width() > max.width() || value.height() > max.height()) size = value.boundedTo(max);
        return size;
    }
};

template <>
struct Converter<QMargins> : DefaultConverter<QMargins> {
    static QJsonValue toJson(const QMargins& value) {
        QJsonObject obj;
        obj["top"] = value.top();
        obj["bottom"] = value.bottom();
        obj["left"] = value.left();
        obj["right"] = value.right();
        return obj;
    }
    static QMargins fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto obj = Converter<QJsonObject>::fromJson(json, errors);
        QMargins margins;
        margins.setTop(obj.value("top").toInt());
        margins.setBottom(obj.value("bottom").toInt());
        margins.setLeft(obj.value("left").toInt());
        margins.setRight(obj.value("right").toInt());
        return margins;
    }
    static QMargins clamp(const QMargins& value, const QMargins& min, const QMargins& max, const QStringList* = nullptr) {
        Q_ASSERT(min.left() <= max.left());
        Q_ASSERT(min.right() <= max.right());
        Q_ASSERT(min.top() <= max.top());
        Q_ASSERT(min.bottom() <= max.bottom());
        QMargins margins = value;
        margins.setLeft(  std::clamp(value.left(),   min.left(),   max.left()));
        margins.setRight( std::clamp(value.right(),  min.right(),  max.right()));
        margins.setTop(   std::clamp(value.top(),    min.top(),    max.top()));
        margins.setBottom(std::clamp(value.bottom(), min.bottom(), max.bottom()));
        return margins;
    }
};

template <>
struct Converter<ScriptSettings> : DefaultConverter<ScriptSettings> {
    static QJsonValue toJson(const ScriptSettings& value) {
        QJsonObject obj;
        obj["path"] = value.path;
        obj["enabled"] = value.enabled;
        return obj;
    }
    static ScriptSettings fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto obj = Converter<QJsonObject>::fromJson(json, errors);
        ScriptSettings settings;
        settings.path = obj.value("path").toString();
        settings.enabled = obj.value("enabled").toBool();
        return settings;
    }
};

template <>
struct Converter<GridSettings> : DefaultConverter<GridSettings> {
    static QJsonValue toJson(const GridSettings& value) {
        return value.toJson();
    }
    static GridSettings fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto obj = Converter<QJsonObject>::fromJson(json, errors);
        return GridSettings::fromJson(obj);
    }
};

template <>
struct Converter<QByteArray> : DefaultConverter<QByteArray> {
    static QJsonValue toJson(const QByteArray& value) {
        return QString::fromLocal8Bit(value.toBase64());
    }
    static QByteArray fromJson(const QJsonValue& json, QStringList* errors = nullptr) {
        const auto s = Converter<QString>::fromJson(json, errors);
        return QByteArray::fromBase64(s.toLocal8Bit());
    }
};

#endif // CONVERTER_H
