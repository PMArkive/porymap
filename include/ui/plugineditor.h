#ifndef PLUGINEDITOR_H
#define PLUGINEDITOR_H

#include <QMainWindow>
#include <QListWidget>
#include <QAbstractButton>
#include <QMessageBox>

#include "pluginlistitem.h"

namespace Ui {
class PluginEditor;
}


class PluginEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit PluginEditor(QWidget *parent = nullptr);
    ~PluginEditor();

signals:
    void reloadScriptEngine();

public slots:
    void applyUserShortcuts();

private:
    Ui::PluginEditor *ui;

    bool hasUnsavedChanges = false;
    const QString baseDir;

    void displayPlugin(const PluginSettings &settings);
    void displayNewPlugin(QString filepath);
    QString choosePlugin(QString dir);
    void removePlugin(QListWidgetItem * item);
    void replacePlugin(QListWidgetItem * item);
    void openPlugin(QListWidgetItem * item);
    QString getPluginFilepath(QListWidgetItem * item, bool absolutePath = true) const;
    void setPluginFilepath(QListWidgetItem * item, QString filepath) const;
    bool getPluginEnabled(QListWidgetItem * item) const;
    void markEdited();
    int prompt(const QString &text, QMessageBox::StandardButton defaultButton);
    void save();
    void closeEvent(QCloseEvent*);
    void initShortcuts();
    QObjectList shortcutableObjects() const;
    void openManual();

private slots:
    void dialogButtonClicked(QAbstractButton *button);
    void createNewPlugin();
    void loadPlugin();
    bool refreshPlugins();
    void userRefreshPlugins();
    void removeSelectedPlugins();
    void openSelectedPlugins();
};

#endif // PLUGINEDITOR_H
