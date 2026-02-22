#include "customscriptslistitem.h"
#include "ui_customscriptslistitem.h"

CustomScriptsListItem::CustomScriptsListItem(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CustomScriptsListItem)
{
    ui->setupUi(this);

    connect(ui->b_Choose, &QAbstractButton::clicked, this, &CustomScriptsListItem::clickedChooseScript);
    connect(ui->b_Edit,   &QAbstractButton::clicked, this, &CustomScriptsListItem::clickedEditScript);
    connect(ui->b_Delete, &QAbstractButton::clicked, this, &CustomScriptsListItem::clickedDeleteScript);
    connect(ui->checkBox_Enable, &QCheckBox::toggled, this, &CustomScriptsListItem::toggledEnable);
    connect(ui->lineEdit_filepath, &QLineEdit::textEdited, this, &CustomScriptsListItem::pathEdited);
}

CustomScriptsListItem::CustomScriptsListItem(const ScriptSettings& settings, QWidget *parent) :
    CustomScriptsListItem(parent)
{
    setSettings(settings);
}

CustomScriptsListItem::~CustomScriptsListItem() {
    delete ui;
}

void CustomScriptsListItem::setPath(const QString& text) {
    ui->lineEdit_filepath->setText(text);
}

QString CustomScriptsListItem::path() const {
    return ui->lineEdit_filepath->text();
}

void CustomScriptsListItem::setScriptEnabled(bool enabled) {
    ui->checkBox_Enable->setChecked(enabled);
}

bool CustomScriptsListItem::scriptEnabled() const {
    return ui->checkBox_Enable->isChecked();
}

// TODO: The two functions below should read/write from/to the UI whether the script belongs to the project or user.
void CustomScriptsListItem::setSettings(const ScriptSettings& settings) {
    setPath(settings.path);
    setScriptEnabled(settings.enabled);
}

ScriptSettings CustomScriptsListItem::getSettings() const {
    return {
        .path = path(),
        .enabled = scriptEnabled(),
        .userOnly = true,
    };
}
