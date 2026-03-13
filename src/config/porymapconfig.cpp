#include "porymapconfig.h"

#include <QMainWindow>
#include <QSplitter>

PorymapConfig porymapConfig;

bool PorymapConfig::save()  {
    // Clean out old rate limit times, leaving only times still in the future.
    for (auto it = this->rateLimitTimes.begin(); it != this->rateLimitTimes.end();) {
        const QDateTime time = it.value();
        if (!time.isNull() && time > QDateTime::currentDateTime()) {
            it = this->rateLimitTimes.erase(it);
        } else it++;
    }

    return KeyValueConfigBase::save();
}

void PorymapConfig::loadFromJson(const QJsonObject& obj) {
    KeyValueConfigBase::loadFromJson(obj);

    // Reset geometry between major/minor versions.
    // We could try to keep separate versions for each geometry,
    // but that requires a lot of careful maintenance.
    // This ensures that as widgets change they won't
    // receive data for old layouts/states, and that as widgets
    // get renamed their old keys wont accumulate in the config.
    constexpr int CurrentGeometryVersion = 1;
    if (this->geometryVersion != CurrentGeometryVersion) {
        this->geometryVersion = CurrentGeometryVersion;
        this->savedGeometryMap.clear();
    }

    this->gridSettings.offsetX = std::clamp(this->gridSettings.offsetX, 0, 999);
    this->gridSettings.offsetY = std::clamp(this->gridSettings.offsetY, 0, 999);
}

QJsonObject PorymapConfig::getDefaultJson() const {
    PorymapConfig defaultConfig;
    return defaultConfig.toJson();
}

void PorymapConfig::addRecentProject(const QString& project) {
    this->recentProjects.removeOne(project);
    this->recentProjects.prepend(project);
}

void PorymapConfig::setRecentProjects(const QStringList& projects) {
    this->recentProjects = projects;
}

QString PorymapConfig::getRecentProject() const {
    return this->recentProjects.value(0);
}

const QStringList& PorymapConfig::getRecentProjects() const {
    return this->recentProjects;
}

QList<QWidget*> PorymapConfig::findChildrenWithStates(const QWidget* widget) {
    QList<QWidget*> children;
    for (const auto& c : widget->findChildren<QSplitter*>()) children.append(qobject_cast<QWidget*>(c));
    for (const auto& c : widget->findChildren<QTabWidget*>()) children.append(qobject_cast<QWidget*>(c));
    return children;
}

void PorymapConfig::saveGeometry(const QWidget* widget, const QString& keyPrefix, bool recursive) {
    if (!widget || widget->objectName().isEmpty()) return;

    const QString key = keyPrefix + widget->objectName();
    if (!widget->parentWidget()) { // Only save geometry for top-level widgets;
        this->savedGeometryMap.insert(key, widget->saveGeometry());
    }

    // In addition to geometry, some widgets have other states that can be saved/restored.
    const QByteArray state = getWidgetState(widget);
    if (!state.isEmpty()) {
        this->savedGeometryMap.insert(key + QStringLiteral("/State"), state);
    }

    if (recursive) {
        for (const auto& child : findChildrenWithStates(widget)) {
            saveGeometry(child, key + "_", false);
        }
    }
}

QByteArray PorymapConfig::getWidgetState(const QWidget* widget) {
    auto mainWindow = qobject_cast<const QMainWindow*>(widget);
    if (mainWindow) return mainWindow->saveState();

    auto splitter = qobject_cast<const QSplitter*>(widget);
    if (splitter) return splitter->saveState();

    auto tabWidget = qobject_cast<const QTabWidget*>(widget);
    if (tabWidget) return QString::number(tabWidget->currentIndex()).toUtf8();

    return QByteArray();
}

bool PorymapConfig::restoreGeometry(QWidget* widget, const QString& keyPrefix, bool recursive) const {
    if (!widget || widget->objectName().isEmpty()) return false;

    const QString key = keyPrefix + widget->objectName();
    auto it = this->savedGeometryMap.constFind(key);
    if (it != this->savedGeometryMap.constEnd()) {
        widget->restoreGeometry(it.value());
    }

    // In addition to geometry, some widgets have other states that can be saved/restored.
    it = this->savedGeometryMap.constFind(key + QStringLiteral("/State"));
    if (it != this->savedGeometryMap.constEnd()) restoreWidgetState(widget, it.value());

    if (recursive) {
        for (const auto& child : findChildrenWithStates(widget)) {
            restoreGeometry(child, key + "_", false);
        }
    }
    return true;
}

void PorymapConfig::restoreWidgetState(QWidget* widget, const QByteArray& state) {
    auto mainWindow = qobject_cast<QMainWindow*>(widget);
    if (mainWindow) {
        mainWindow->restoreState(state);
        return;
    }
    auto splitter = qobject_cast<QSplitter*>(widget);
    if (splitter) {
        splitter->restoreState(state);
        return;
    }
    auto tabWidget = qobject_cast<QTabWidget*>(widget);
    if (tabWidget) {
        tabWidget->setCurrentIndex(QString::fromUtf8(state).toInt());
        return;
    }
}
