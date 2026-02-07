#include "customscriptseditor.h"
#include "ui_customscriptseditor.h"
#include "ui_customscriptslistitem.h"
#include "config.h"
#include "editor.h"
#include "shortcut.h"
#include "filedialog.h"
#include "eventfilters.h"

#include <QDir>

CustomScriptsEditor::CustomScriptsEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CustomScriptsEditor),
    baseDir(userConfig.projectDir() + "/")
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    // This property seems to be reset if we don't set it programmatically
    ui->list->setDragDropMode(QAbstractItemView::NoDragDrop);

    for (const auto& settings : projectConfig.customScripts)
        displayScript(settings);
    for (const auto& settings : userConfig.customScripts)
        displayScript(settings);

    connect(ui->button_Help, &QAbstractButton::clicked, this, &CustomScriptsEditor::openManual);
    connect(ui->button_CreateNewScript, &QAbstractButton::clicked, this, &CustomScriptsEditor::createNewScript);
    connect(ui->button_LoadScript, &QAbstractButton::clicked, this, &CustomScriptsEditor::loadScript);
    connect(ui->button_RefreshScripts, &QAbstractButton::clicked, this, &CustomScriptsEditor::userRefreshScripts);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &CustomScriptsEditor::dialogButtonClicked);

    initShortcuts();
    installEventFilter(new GeometrySaver(this));
}

CustomScriptsEditor::~CustomScriptsEditor()
{
    ui->list->clear();
    delete ui;
}

void CustomScriptsEditor::initShortcuts() {
    auto *shortcut_remove = new Shortcut({QKeySequence("Del"), QKeySequence("Backspace")}, this, SLOT(removeSelectedScripts()));
    shortcut_remove->setObjectName("shortcut_remove");
    shortcut_remove->setWhatsThis("Remove Selected Scripts");

    auto *shortcut_open = new Shortcut(QKeySequence(), this, SLOT(openSelectedScripts()));
    shortcut_open->setObjectName("shortcut_open");
    shortcut_open->setWhatsThis("Open Selected Scripts");

    auto *shortcut_createNew = new Shortcut(QKeySequence(), this, SLOT(createNewScript()));
    shortcut_createNew->setObjectName("shortcut_createNew");
    shortcut_createNew->setWhatsThis("Create New Script...");

    auto *shortcut_load = new Shortcut(QKeySequence(), this, SLOT(loadScript()));
    shortcut_load->setObjectName("shortcut_load");
    shortcut_load->setWhatsThis("Load Script...");

    auto *shortcut_refresh = new Shortcut(QKeySequence(), this, SLOT(userRefreshScripts()));
    shortcut_refresh->setObjectName("shortcut_refresh");
    shortcut_refresh->setWhatsThis("Refresh Scripts");

    shortcutsConfig.setDefaultShortcuts(shortcutableObjects());
    applyUserShortcuts();
}

QObjectList CustomScriptsEditor::shortcutableObjects() const {
    QObjectList shortcutable_objects;

    for (auto *action : findChildren<QAction *>())
        if (!action->objectName().isEmpty())
            shortcutable_objects.append(qobject_cast<QObject *>(action));
    for (auto *shortcut : findChildren<Shortcut *>())
        if (!shortcut->objectName().isEmpty())
            shortcutable_objects.append(qobject_cast<QObject *>(shortcut));

    return shortcutable_objects;
}

void CustomScriptsEditor::applyUserShortcuts() {
    for (auto *action : findChildren<QAction *>())
        if (!action->objectName().isEmpty())
            action->setShortcuts(shortcutsConfig.userShortcuts(action));
    for (auto *shortcut : findChildren<Shortcut *>())
        if (!shortcut->objectName().isEmpty())
            shortcut->setKeys(shortcutsConfig.userShortcuts(shortcut));
}

void CustomScriptsEditor::displayScript(const ScriptSettings& settings) {
    auto item = new QListWidgetItem(ui->list);
    auto widget = new CustomScriptsListItem(settings, ui->list);
    item->setSizeHint(widget->sizeHint());

    connect(widget, &CustomScriptsListItem::clickedChooseScript, [this, item] { this->replaceScript(item); });
    connect(widget, &CustomScriptsListItem::clickedEditScript,   [this, item] { this->openScript(item); });
    connect(widget, &CustomScriptsListItem::clickedDeleteScript, [this, item] { this->removeScript(item); });
    connect(widget, &CustomScriptsListItem::toggledEnable, this, &CustomScriptsEditor::markEdited);
    connect(widget, &CustomScriptsListItem::pathEdited, this, &CustomScriptsEditor::markEdited);

    // Per the Qt manual, for performance reasons QListWidget::setItemWidget shouldn't be used with non-static items.
    // There's an assumption here that users won't have enough scripts for that to be a problem.
    ui->list->addItem(item);
    ui->list->setItemWidget(item, widget);
}

void CustomScriptsEditor::markEdited() {
    this->hasUnsavedChanges = true;
}

QString CustomScriptsEditor::getScriptFilepath(QListWidgetItem * item, bool absolutePath) const {
    auto widget = dynamic_cast<CustomScriptsListItem *>(ui->list->itemWidget(item));
    if (!widget) return QString();

    QString path = widget->path();
    if (absolutePath) {
        QFileInfo fileInfo(path);
        if (fileInfo.isRelative())
            path.prepend(this->baseDir);
    }
    return path;
}

void CustomScriptsEditor::setScriptFilepath(QListWidgetItem * item, QString filepath) const {
    auto widget = dynamic_cast<CustomScriptsListItem *>(ui->list->itemWidget(item));
    if (widget) {
        widget->setPath(Util::stripPrefix(filepath, this->baseDir));
    }
}

bool CustomScriptsEditor::getScriptEnabled(QListWidgetItem * item) const {
    auto widget = dynamic_cast<CustomScriptsListItem *>(ui->list->itemWidget(item));
    return widget && widget->scriptEnabled();
}

QString CustomScriptsEditor::chooseScript(QString dir) {
    return FileDialog::getOpenFileName(this, "Choose Custom Script File", dir, "JavaScript Files (*.js)");
}

void CustomScriptsEditor::createNewScript() {
    const QString filepath = FileDialog::getSaveFileName(this, "Create New Script File", FileDialog::getDirectory() + "/new_script.js", "JavaScript Files (*.js)");
    if (filepath.isEmpty())
        return;

    QFile scriptFile(filepath);
    if (!scriptFile.open(QIODevice::WriteOnly)) {
        logError(QString("Error: Could not open %1 for writing").arg(filepath));
        QMessageBox messageBox(this);
        messageBox.setText("Failed to create new script file!");
        messageBox.setInformativeText(QString("Could not open \"%1\" for writing").arg(filepath));
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.exec();
        return;
    }
    ParseUtil parser;
    scriptFile.write(parser.readTextFile(":/text/script_template.txt").toUtf8());
    scriptFile.close();

    this->displayNewScript(filepath);
}

void CustomScriptsEditor::loadScript() {
    QString filepath = this->chooseScript(FileDialog::getDirectory());
    if (filepath.isEmpty())
        return;
    this->displayNewScript(filepath);
}

void CustomScriptsEditor::displayNewScript(QString filepath) {
    filepath = Util::stripPrefix(filepath, this->baseDir);

    // Verify new script path is not already in list
    for (int i = 0; i < ui->list->count(); i++) {
        if (filepath == this->getScriptFilepath(ui->list->item(i), false)) {
            QMessageBox::information(this, QApplication::applicationName(), QString("The script '%1' is already loaded").arg(filepath));
            return;
        }
    }

    ScriptSettings settings;
    settings.path = filepath;
    this->displayScript(settings);
    this->markEdited();
}

void CustomScriptsEditor::removeScript(QListWidgetItem * item) {
    ui->list->takeItem(ui->list->row(item));
    this->markEdited();
}

void CustomScriptsEditor::removeSelectedScripts() {
    QList<QListWidgetItem *> items = ui->list->selectedItems();
    if (items.length() == 0)
        return;
    for (auto item : items)
        this->removeScript(item);
}

void CustomScriptsEditor::replaceScript(QListWidgetItem * item) {
    const QString filepath = this->chooseScript(this->getScriptFilepath(item));
    if (filepath.isEmpty())
        return;
    this->setScriptFilepath(item, filepath);
    this->markEdited();
}

void CustomScriptsEditor::openScript(QListWidgetItem * item) {
    const QString path = this->getScriptFilepath(item);
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile()){
        QMessageBox::warning(this, QApplication::applicationName(), QString("Failed to open script '%1'").arg(path));
        return;
    }
    Editor::openInTextEditor(path);
}

void CustomScriptsEditor::openSelectedScripts() {
    for (auto item : ui->list->selectedItems())
        this->openScript(item);
}

void CustomScriptsEditor::openManual() {
    static const QUrl url("https://huderlem.github.io/porymap/manual/scripting-capabilities.html");
    QDesktopServices::openUrl(url);
}

// When the user refreshes the scripts we show a little tooltip as feedback.
// We don't want this tooltip to display when we refresh programmatically, like when changes are saved.
void CustomScriptsEditor::userRefreshScripts() {
    if (refreshScripts())
        QToolTip::showText(ui->button_RefreshScripts->mapToGlobal(QPoint(0, 0)), "Refreshed!");
}

bool CustomScriptsEditor::refreshScripts() {
    if (this->hasUnsavedChanges) {
        if (this->prompt("Scripts have been modified, save changes and reload the script engine?", QMessageBox::Yes) == QMessageBox::No)
            return false;
        this->save();
    }
    emit reloadScriptEngine();
    return true;
}

void CustomScriptsEditor::save() {
    if (!this->hasUnsavedChanges)
        return;

    QList<ScriptSettings> userScripts;
    QList<ScriptSettings> projectScripts;
    for (int i = 0; i < ui->list->count(); i++) {
        auto item = ui->list->item(i);
        auto widget = dynamic_cast<CustomScriptsListItem *>(ui->list->itemWidget(item));
        if (!widget) continue;
        const ScriptSettings settings = widget->getSettings();
        if (settings.userOnly) userScripts.append(settings);
        else projectScripts.append(settings);
    }

    userConfig.customScripts = userScripts;
    userConfig.save();

    projectConfig.customScripts = projectScripts;
    projectConfig.save();

    this->hasUnsavedChanges = false;
    this->refreshScripts();
}

int CustomScriptsEditor::prompt(const QString &text, QMessageBox::StandardButton defaultButton) {
    QMessageBox messageBox(this);
    messageBox.setText(text);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | defaultButton);
    messageBox.setDefaultButton(defaultButton);
    return messageBox.exec();
}

void CustomScriptsEditor::dialogButtonClicked(QAbstractButton *button) {
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
        this->save();
    close(); // All buttons (OK and Cancel) close the window
}

void CustomScriptsEditor::closeEvent(QCloseEvent* event) {
    if (this->hasUnsavedChanges) {
        int result = this->prompt("Scripts have been modified, save changes?", QMessageBox::Cancel);
        if (result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (result == QMessageBox::Yes)
            this->save();
    }
}
