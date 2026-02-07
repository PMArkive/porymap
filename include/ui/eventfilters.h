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


/// Emits a signal when a window gets activated / regains focus
class ActiveWindowFilter : public QObject {
    Q_OBJECT
public:
    ActiveWindowFilter(QObject *parent) : QObject(parent) {}
    virtual ~ActiveWindowFilter() {}
    bool eventFilter(QObject *obj, QEvent *event) override;
signals:
    void activated();
};


class GeometrySaver : public QObject {
    Q_OBJECT
public:
    GeometrySaver(QObject *parent, bool enableLogging = true)
        : QObject(parent), m_loggingEnabled(enableLogging) {}
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    bool m_loggingEnabled = true;
    QSet<QObject*> m_wasShown;
};
