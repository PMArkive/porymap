#include "eventfilters.h"
#include "config.h"
#include "log.h"

#include <QGraphicsSceneWheelEvent>


bool MapSceneEventFilter::eventFilter(QObject*, QEvent *event) {
    if (event->type() == QEvent::GraphicsSceneWheel) {
        QGraphicsSceneWheelEvent *wheelEvent = static_cast<QGraphicsSceneWheelEvent *>(event);
        if (wheelEvent->modifiers() & Qt::ControlModifier) {
            emit wheelZoom(wheelEvent->delta() > 0 ? 1 : -1);
            event->accept();
            return true;
        }
    }
    return false;
}


bool ActiveWindowFilter::eventFilter(QObject*, QEvent *event) {
    if (event->type() == QEvent::WindowActivate) {
        emit activated();
    }
    return false;
}


bool GeometrySaver::eventFilter(QObject *object, QEvent *event) {
    if (event->spontaneous()) return false;

    auto w = qobject_cast<QWidget*>(object);
    if (!w) return false;

    if (event->type() == QEvent::Polish) {
        // Note: Restoring geometry in QEvent::Show would be too late,
        //       and the widget would briefly appear with the old geometry.
        porymapConfig.restoreGeometry(w);
    } else if (event->type() == QEvent::Show) {
        if (m_loggingEnabled && !w->windowTitle().isEmpty()) {
            logInfo(QString("Opening window: %1").arg(w->windowTitle()));
        }
        m_wasShown.insert(object);
    } else if (event->type() == QEvent::Close && m_wasShown.contains(object)) {
        // There are situations where a window might be 'closed' without
        // ever actually having been opened (for example, the Shortcuts Editor
        // will quietly construct windows to get their shortcuts, and those windows
        // can later be closed without having been displayed).
        // We don't want to save the geometry of these windows, or log that they closed,
        // so we've checked to make sure the widget was displayed before proceeding.
        porymapConfig.saveGeometry(w);
        if (m_loggingEnabled && !w->windowTitle().isEmpty()) {
            logInfo(QString("Closing window: %1").arg(w->windowTitle()));
        }
    }
    return false;
}