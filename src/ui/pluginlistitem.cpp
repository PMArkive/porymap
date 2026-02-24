#include "pluginlistitem.h"
#include "ui_pluginlistitem.h"

PluginListItem::PluginListItem(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PluginListItem)
{
    ui->setupUi(this);

    connect(ui->b_Choose, &QAbstractButton::clicked, this, &PluginListItem::clickedChoosePlugin);
    connect(ui->b_Edit,   &QAbstractButton::clicked, this, &PluginListItem::clickedEditPlugin);
    connect(ui->b_Delete, &QAbstractButton::clicked, this, &PluginListItem::clickedDeletePlugin);
    connect(ui->checkBox_Enable, &QCheckBox::toggled, this, &PluginListItem::toggledEnable);
    connect(ui->lineEdit_filepath, &QLineEdit::textEdited, this, &PluginListItem::pathEdited);
}

PluginListItem::PluginListItem(const PluginSettings& settings, QWidget *parent) :
    PluginListItem(parent)
{
    setSettings(settings);
}

PluginListItem::~PluginListItem() {
    delete ui;
}

void PluginListItem::setPath(const QString& text) {
    ui->lineEdit_filepath->setText(text);
}

QString PluginListItem::path() const {
    return ui->lineEdit_filepath->text();
}

void PluginListItem::setPluginEnabled(bool enabled) {
    ui->checkBox_Enable->setChecked(enabled);
}

bool PluginListItem::pluginEnabled() const {
    return ui->checkBox_Enable->isChecked();
}

// TODO: The two functions below should read/write from/to the UI whether the plugin belongs to the project or user.
void PluginListItem::setSettings(const PluginSettings& settings) {
    setPath(settings.path);
    setPluginEnabled(settings.enabled);
}

PluginSettings PluginListItem::getSettings() const {
    return {
        .path = path(),
        .enabled = pluginEnabled(),
        .userOnly = true,
    };
}
