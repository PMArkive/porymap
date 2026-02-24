#include "plugineditor.h"
#include "ui_plugineditor.h"
#include "config.h"
#include "editor.h"
#include "shortcut.h"
#include "filedialog.h"
#include "eventfilters.h"

#include <QDesktopServices>
#include <QDir>
#include <QToolTip>

PluginEditor::PluginEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PluginEditor),
    baseDir(userConfig.projectDir() + "/")
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    // This property seems to be reset if we don't set it programmatically
    ui->list->setDragDropMode(QAbstractItemView::NoDragDrop);

    for (const auto& plugin : projectConfig.plugins)
        displayPlugin(plugin);
    for (const auto& plugin : userConfig.plugins)
        displayPlugin(plugin);

    connect(ui->button_Help, &QAbstractButton::clicked, this, &PluginEditor::openManual);
    connect(ui->button_CreateNewPlugin, &QAbstractButton::clicked, this, &PluginEditor::createNewPlugin);
    connect(ui->button_LoadPlugin, &QAbstractButton::clicked, this, &PluginEditor::loadPlugin);
    connect(ui->button_RefreshPlugins, &QAbstractButton::clicked, this, &PluginEditor::userRefreshPlugins);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &PluginEditor::dialogButtonClicked);

    initShortcuts();
    installEventFilter(new GeometrySaver(this));
}

PluginEditor::~PluginEditor()
{
    ui->list->clear();
    delete ui;
}

void PluginEditor::initShortcuts() {
    auto *shortcut_remove = new Shortcut({QKeySequence("Del"), QKeySequence("Backspace")}, this, SLOT(removeSelectedPlugins()));
    shortcut_remove->setObjectName("shortcut_remove");
    shortcut_remove->setWhatsThis("Remove Selected Plugins");

    auto *shortcut_open = new Shortcut(QKeySequence(), this, SLOT(openSelectedPlugins()));
    shortcut_open->setObjectName("shortcut_open");
    shortcut_open->setWhatsThis("Open Selected Plugins");

    auto *shortcut_createNew = new Shortcut(QKeySequence(), this, SLOT(createNewPlugin()));
    shortcut_createNew->setObjectName("shortcut_createNew");
    shortcut_createNew->setWhatsThis("Create New Plugin...");

    auto *shortcut_load = new Shortcut(QKeySequence(), this, SLOT(loadPlugin()));
    shortcut_load->setObjectName("shortcut_load");
    shortcut_load->setWhatsThis("Load Plugin...");

    auto *shortcut_refresh = new Shortcut(QKeySequence(), this, SLOT(userRefreshPlugins()));
    shortcut_refresh->setObjectName("shortcut_refresh");
    shortcut_refresh->setWhatsThis("Refresh Plugins");

    shortcutsConfig.setDefaultShortcuts(shortcutableObjects());
    applyUserShortcuts();
}

QObjectList PluginEditor::shortcutableObjects() const {
    QObjectList shortcutable_objects;

    for (auto *action : findChildren<QAction *>())
        if (!action->objectName().isEmpty())
            shortcutable_objects.append(qobject_cast<QObject *>(action));
    for (auto *shortcut : findChildren<Shortcut *>())
        if (!shortcut->objectName().isEmpty())
            shortcutable_objects.append(qobject_cast<QObject *>(shortcut));

    return shortcutable_objects;
}

void PluginEditor::applyUserShortcuts() {
    for (auto *action : findChildren<QAction *>())
        if (!action->objectName().isEmpty())
            action->setShortcuts(shortcutsConfig.userShortcuts(action));
    for (auto *shortcut : findChildren<Shortcut *>())
        if (!shortcut->objectName().isEmpty())
            shortcut->setKeys(shortcutsConfig.userShortcuts(shortcut));
}

void PluginEditor::displayPlugin(const PluginSettings& settings) {
    auto item = new QListWidgetItem(ui->list);
    auto widget = new PluginListItem(settings, ui->list);
    item->setSizeHint(widget->sizeHint());

    connect(widget, &PluginListItem::clickedChoosePlugin, [this, item] { this->replacePlugin(item); });
    connect(widget, &PluginListItem::clickedEditPlugin,   [this, item] { this->openPlugin(item); });
    connect(widget, &PluginListItem::clickedDeletePlugin, [this, item] { this->removePlugin(item); });
    connect(widget, &PluginListItem::toggledEnable, this, &PluginEditor::markEdited);
    connect(widget, &PluginListItem::pathEdited, this, &PluginEditor::markEdited);

    // Per the Qt manual, for performance reasons QListWidget::setItemWidget shouldn't be used with non-static items.
    // There's an assumption here that users won't have enough plugins for that to be a problem.
    ui->list->addItem(item);
    ui->list->setItemWidget(item, widget);
}

void PluginEditor::markEdited() {
    this->hasUnsavedChanges = true;
}

QString PluginEditor::getPluginFilepath(QListWidgetItem * item, bool absolutePath) const {
    auto widget = dynamic_cast<PluginListItem *>(ui->list->itemWidget(item));
    if (!widget) return QString();

    QString path = widget->path();
    if (absolutePath) {
        QFileInfo fileInfo(path);
        if (fileInfo.isRelative())
            path.prepend(this->baseDir);
    }
    return path;
}

void PluginEditor::setPluginFilepath(QListWidgetItem * item, QString filepath) const {
    auto widget = dynamic_cast<PluginListItem *>(ui->list->itemWidget(item));
    if (widget) {
        widget->setPath(Util::stripPrefix(filepath, this->baseDir));
    }
}

bool PluginEditor::getPluginEnabled(QListWidgetItem * item) const {
    auto widget = dynamic_cast<PluginListItem *>(ui->list->itemWidget(item));
    return widget && widget->pluginEnabled();
}

QString PluginEditor::choosePlugin(QString dir) {
    return FileDialog::getOpenFileName(this, "Choose Plugin", dir, "JavaScript Files (*.js)");
}

void PluginEditor::createNewPlugin() {
    const QString filepath = FileDialog::getSaveFileName(this, "Create New Plugin", FileDialog::getDirectory() + "/new_plugin.js", "JavaScript Files (*.js)");
    if (filepath.isEmpty())
        return;

    QFile pluginFile(filepath);
    if (!pluginFile.open(QIODevice::WriteOnly)) {
        logError(QString("Error: Could not open %1 for writing").arg(filepath));
        QMessageBox messageBox(this);
        messageBox.setText("Failed to create new plugin file!");
        messageBox.setInformativeText(QString("Could not open \"%1\" for writing").arg(filepath));
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.exec();
        return;
    }
    ParseUtil parser;
    pluginFile.write(parser.readTextFile(":/text/plugin_template.txt").toUtf8());
    pluginFile.close();

    this->displayNewPlugin(filepath);
}

void PluginEditor::loadPlugin() {
    QString filepath = this->choosePlugin(FileDialog::getDirectory());
    if (filepath.isEmpty())
        return;
    this->displayNewPlugin(filepath);
}

void PluginEditor::displayNewPlugin(QString filepath) {
    filepath = Util::stripPrefix(filepath, this->baseDir);

    // Verify new plugin path is not already in list
    for (int i = 0; i < ui->list->count(); i++) {
        if (filepath == this->getPluginFilepath(ui->list->item(i), false)) {
            QMessageBox::information(this, QApplication::applicationName(), QString("The plugin '%1' is already loaded").arg(filepath));
            return;
        }
    }

    PluginSettings settings;
    settings.path = filepath;
    this->displayPlugin(settings);
    this->markEdited();
}

void PluginEditor::removePlugin(QListWidgetItem * item) {
    ui->list->takeItem(ui->list->row(item));
    this->markEdited();
}

void PluginEditor::removeSelectedPlugins() {
    QList<QListWidgetItem *> items = ui->list->selectedItems();
    if (items.length() == 0)
        return;
    for (auto item : items)
        this->removePlugin(item);
}

void PluginEditor::replacePlugin(QListWidgetItem * item) {
    const QString filepath = this->choosePlugin(this->getPluginFilepath(item));
    if (filepath.isEmpty())
        return;
    this->setPluginFilepath(item, filepath);
    this->markEdited();
}

void PluginEditor::openPlugin(QListWidgetItem * item) {
    const QString path = this->getPluginFilepath(item);
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile()){
        QMessageBox::warning(this, QApplication::applicationName(), QString("Failed to open plugin '%1'").arg(path));
        return;
    }
    Editor::openInTextEditor(path);
}

void PluginEditor::openSelectedPlugins() {
    for (auto item : ui->list->selectedItems())
        this->openPlugin(item);
}

void PluginEditor::openManual() {
    static const QUrl url("https://huderlem.github.io/porymap/manual/scripting-capabilities.html");
    QDesktopServices::openUrl(url);
}

// When the user refreshes the plugins we show a little tooltip as feedback.
// We don't want this tooltip to display when we refresh programmatically, like when changes are saved.
void PluginEditor::userRefreshPlugins() {
    if (refreshPlugins())
        QToolTip::showText(ui->button_RefreshPlugins->mapToGlobal(QPoint(0, 0)), "Refreshed!");
}

bool PluginEditor::refreshPlugins() {
    if (this->hasUnsavedChanges) {
        if (this->prompt("Plugins have been modified, save changes and reload plugins?", QMessageBox::Yes) == QMessageBox::No)
            return false;
        this->save();
    }
    emit reloadScriptEngine();
    return true;
}

void PluginEditor::save() {
    if (!this->hasUnsavedChanges)
        return;

    QList<PluginSettings> userPlugins;
    QList<PluginSettings> projectPlugins;
    for (int i = 0; i < ui->list->count(); i++) {
        auto item = ui->list->item(i);
        auto widget = dynamic_cast<PluginListItem *>(ui->list->itemWidget(item));
        if (!widget) continue;
        const PluginSettings settings = widget->getSettings();
        if (settings.userOnly) userPlugins.append(settings);
        else projectPlugins.append(settings);
    }

    userConfig.plugins = userPlugins;
    userConfig.save();

    projectConfig.plugins = projectPlugins;
    projectConfig.save();

    this->hasUnsavedChanges = false;
    this->refreshPlugins();
}

int PluginEditor::prompt(const QString &text, QMessageBox::StandardButton defaultButton) {
    QMessageBox messageBox(this);
    messageBox.setText(text);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | defaultButton);
    messageBox.setDefaultButton(defaultButton);
    return messageBox.exec();
}

void PluginEditor::dialogButtonClicked(QAbstractButton *button) {
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
        this->save();
    close(); // All buttons (OK and Cancel) close the window
}

void PluginEditor::closeEvent(QCloseEvent* event) {
    if (this->hasUnsavedChanges) {
        int result = this->prompt("Plugins have been modified, save changes?", QMessageBox::Cancel);
        if (result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (result == QMessageBox::Yes)
            this->save();
    }
}
