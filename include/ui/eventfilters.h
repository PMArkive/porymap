#include <QObject>
#include <QEvent>
#include <QSet>


/// Ctrl+Wheel = zoom
class MapSceneEventFilter : public QObject {
    Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
public:
    explicit MapSceneEventFilter(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void wheelZoom(int delta);
public slots:
};


/// Emits a signal when the given event or events occur
class EventSignaler : public QObject {
    Q_OBJECT
public:
    EventSignaler(const QSet<QEvent::Type>& types, QObject *parent) : QObject(parent), m_types(types) {}
    EventSignaler(QEvent::Type type, QObject *parent) : EventSignaler(QSet<QEvent::Type>{type}, parent) {}
    virtual ~EventSignaler() {}

    void addEventType(QEvent::Type type) { m_types.insert(type); }
    void removeEventType(QEvent::Type type) { m_types.remove(type); }

    bool eventFilter(QObject *obj, QEvent *event) override;
signals:
    void triggered();
private:
    QSet<QEvent::Type> m_types;
};


class GeometrySaver : public QObject {
    Q_OBJECT
public:
    GeometrySaver(QObject *parent, bool enableLogging = true)
        : QObject(parent), m_loggingEnabled(enableLogging) {}
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    bool m_loggingEnabled = true;
    QSet<QObject*> m_shown;
};
