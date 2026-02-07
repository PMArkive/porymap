#ifndef CUSTOMSCRIPTSLISTITEM_H
#define CUSTOMSCRIPTSLISTITEM_H

#include <QFrame>
#include "scriptsettings.h"

namespace Ui {
class CustomScriptsListItem;
}

class CustomScriptsListItem : public QFrame
{
    Q_OBJECT

public:
    explicit CustomScriptsListItem(QWidget *parent = nullptr);
    explicit CustomScriptsListItem(const ScriptSettings& settings, QWidget *parent = nullptr);
    ~CustomScriptsListItem();

    void setSettings(const ScriptSettings& settings);
    ScriptSettings getSettings() const;

    void setPath(const QString& text);
    QString path() const;

    void setScriptEnabled(bool enabled);
    bool scriptEnabled() const;

signals:
    void clickedChooseScript();
    void clickedEditScript();
    void clickedDeleteScript();
    void toggledEnable(bool checked);
    void pathEdited(const QString& text);

private:
    Ui::CustomScriptsListItem *ui;
};

#endif // CUSTOMSCRIPTSLISTITEM_H
