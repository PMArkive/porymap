#include "converter.h"

// A FieldInterface provides a simple interface for converting a field to/from JSON.
// It's constructed with a pointer to some data, and has two functions:
// - 'get' returns the pointed-to data, converted to JSON
// - 'set' assigns the pointed-to data to a given QJsonValue, with appropriate conversion.
//   Returns any errors that occur during conversion/assignment.
//
// A FieldInterface is normally constructed using the 'makeFieldInterface' function:
// Example:
//    int someField = 0;
//    FieldInterface* fi = makeFieldInterface(&someField);
//    fi->set(QJsonValue("5")); // someField is now 5
//
// There are additional implementations of 'makeFieldInterface' that let you specify what valid values are.
// Example:
//    int someField = 0;
//    int min = 1, max = 4;
//    FieldInterface* fi = makeFieldInterface(&someField, min, max);
//    fi->set(QJsonValue("5")); // someField is now 4 (defaults to closest bound), error messages returned
// or
//    QString someField = "";
//    QList<QString> options = {"hello","hi there"};
//    FieldInterface* fi = makeFieldInterface(&someField, options);
//    fi->set(QJsonValue("5")); // someField is now "hello" (defaults to first element), error messages returned

// Base class lets us use the interface without any type information.
class FieldInterface {
public:
    FieldInterface(){};
    virtual ~FieldInterface() {};
    virtual QJsonValue get() const = 0;
    virtual QStringList set(const QJsonValue& json) const = 0;
};

template <typename T>
class BasicFieldInterface : public FieldInterface {
public:
    BasicFieldInterface(T* field) : m_field(field) {
        Q_ASSERT(m_field);
    };
    virtual ~BasicFieldInterface() {};
    virtual QJsonValue get() const override {return Converter<T>::toJson(*m_field);}
    virtual QStringList set(const QJsonValue& json) const override {
        QStringList errors;
        auto value = Converter<T>::fromJson(json, &errors);
        if (errors.isEmpty()) *m_field = value; // Don't bother changing the value if conversion failed
        return errors;
    }
protected:
    T* m_field;
};

template <typename T>
static FieldInterface* makeFieldInterface(T* field) {
    return new BasicFieldInterface<T>(field);
}

// Create a regular FieldInterface, but override 'set' to use the given min/max.
template <typename T>
static FieldInterface* makeFieldInterface(T* field, const T& min, const T& max) {
    class BoundedFieldInterface : public BasicFieldInterface<T> {
    public:
        BoundedFieldInterface(T* field, const T& min, const T& max)
        : BasicFieldInterface<T>(field), m_min(min), m_max(max) {};
        virtual ~BoundedFieldInterface() {};
        virtual QStringList set(const QJsonValue& json) const override {
            QStringList errors;
            auto value = Converter<T>::fromJson(json, &errors);
            if (errors.isEmpty()) { // Don't bother changing the value if conversion failed
                value = Converter<T>::clamp(value, m_min, m_max, &errors);
                *this->m_field = value;
            }
            return errors;
        }
    private:
        const T m_min;
        const T m_max;
    };
    return new BoundedFieldInterface(field, min, max);
}

// Create a regular FieldInterface, but override 'set' to use the given 'acceptableValues'.
template <typename T>
static FieldInterface* makeFieldInterface(T* field, const QList<T>& acceptableValues) {
    Q_ASSERT(!acceptableValues.isEmpty());
    class BoundedFieldInterface : public BasicFieldInterface<T> {
    public:
        BoundedFieldInterface(T* field, const QList<T>& acceptableValues)
        : BasicFieldInterface<T>(field),
         m_acceptableValues(acceptableValues.begin(), acceptableValues.end()),
         m_defaultValue(acceptableValues.first()) {};
        virtual ~BoundedFieldInterface() {};
        virtual QStringList set(const QJsonValue& json) const override {
            QStringList errors;
            auto value = Converter<T>::fromJson(json, &errors);
            if (errors.isEmpty()) {
                if (!m_acceptableValues.contains(value)) {
                    value = m_defaultValue;
                    errors.append("Invalid value.");
                }
                *this->m_field = value;
            }
            return errors;
        }
    private:
        // The order of the list only matters for determining the default value,
        // so save that separately and convert the list to a set for better lookup speed.
        const QSet<T> m_acceptableValues;
        const T m_defaultValue;
    };
    return new BoundedFieldInterface(field, acceptableValues);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

// FieldManager manages a QHash mapping string keys to FieldInterfaces.
// This makes it easy to map many fields to/from JSON without explicitly serializing anything.

class FieldManager {
public:
    ~FieldManager(){ clear(); }

    QStringList setField(const QString& key, const QJsonValue& value) const {
        auto it = m_fields.find(key);
        return (it != m_fields.end()) ? it.value()->set(value) : QStringList();
    }

    QJsonValue getField(const QString& key) const {
        auto it = m_fields.find(key);
        return (it != m_fields.end()) ? it.value()->get() : QJsonValue();
    }

    QJsonObject getFields() const {
        QJsonObject obj;
        for (auto it = m_fields.constBegin(); it != m_fields.constEnd(); it++) {
            obj[it.key()] = it.value()->get();
        }
        return obj;
    }

    void clear() {
        qDeleteAll(m_fields);
        m_fields.clear();
    }

    bool hasField(const QString& key) const {return m_fields.contains(key);}
    
    template <typename T>
    void addField(T* field, const QString& key) {
        Q_ASSERT(!m_fields.contains(key));
        m_fields.insert(key, makeFieldInterface<T>(field));
    }
    template <typename T>
    void addField(T* field, const QString& key, const T& min, const T& max) {
        Q_ASSERT(!m_fields.contains(key));
        m_fields.insert(key, makeFieldInterface<T>(field, min, max));
    }
    template <typename T>
    void addField(T* field, const QString& key, const QList<T>& acceptableValues) {
        Q_ASSERT(!m_fields.contains(key));
        m_fields.insert(key, makeFieldInterface<T>(field, acceptableValues));
    }

private:
    QHash<QString, FieldInterface*> m_fields;
};
