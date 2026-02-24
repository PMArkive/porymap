#ifndef PLUGINLISTITEM_H
#define PLUGINLISTITEM_H

#include <QFrame>
#include "pluginsettings.h"

namespace Ui {
class PluginListItem;
}

class PluginListItem : public QFrame
{
    Q_OBJECT

public:
    explicit PluginListItem(QWidget *parent = nullptr);
    explicit PluginListItem(const PluginSettings& settings, QWidget *parent = nullptr);
    ~PluginListItem();

    void setSettings(const PluginSettings& settings);
    PluginSettings getSettings() const;

    void setPath(const QString& text);
    QString path() const;

    void setPluginEnabled(bool enabled);
    bool pluginEnabled() const;

signals:
    void clickedChoosePlugin();
    void clickedEditPlugin();
    void clickedDeletePlugin();
    void toggledEnable(bool checked);
    void pathEdited(const QString& text);

private:
    Ui::PluginListItem *ui;
};

#endif // PLUGINLISTITEM_H
