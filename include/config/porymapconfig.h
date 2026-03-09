#pragma once
#ifndef PORYMAPCONFIG_H
#define PORYMAPCONFIG_H

#include "keyvalueconfigbase.h"
#include "block.h"
#include "events.h"
#include "log.h"
#include "utility.h"

#include <QColorSpace>
#include <QFontDatabase>
#include <QGraphicsPixmapItem>
#include <QStandardPaths>

enum ScriptAutocompleteMode {
    MapOnly,
    MapAndCommon,
    All,
};

class PorymapConfig: public KeyValueConfigBase
{
public:
    PorymapConfig() : KeyValueConfigBase(QStringLiteral("settings.json")) {
        setRoot(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

        // Initialize defaults not available at compile time.
        this->mapListFont = defaultMapListFont();
    }

    virtual bool save() override;

    virtual void loadFromJson(const QJsonObject& obj) override;

    void addRecentProject(const QString& project);
    void setRecentProjects(const QStringList& projects);
    QString getRecentProject() const;
    const QStringList& getRecentProjects() const;

    void saveGeometry(const QWidget* widget, const QString& keyPrefix = QString(), bool recursive = true);
    bool restoreGeometry(QWidget* widget, const QString& keyPrefix = QString(), bool recursive = true) const;

    static QFont defaultMapListFont() { return QFontDatabase::systemFont(QFontDatabase::FixedFont); }

    bool reopenOnLaunch = true;
    bool projectManuallyClosed = false;
    int mapListTab = 0;
    bool mapListEditGroupsEnabled = false;
    OrderedSet<int> mapListTabsHidingEmptyFolders;
    bool mapListLayoutsSorted = true;
    bool mapListLocationsSorted = true;
    bool prettyCursors = true;
    bool mirrorConnectingMaps = true;
    bool showDiveEmergeMaps = false;
    int diveEmergeMapOpacity = 30;
    int diveMapOpacity = 15;
    int emergeMapOpacity = 15;
    int collisionOpacity = 50;
    int collisionZoom = 30;
    int metatilesZoom = 30;
    int tilesetEditorMetatilesZoom = 30;
    int tilesetEditorTilesZoom = 30;
    Qt::Orientation tilesetEditorLayerOrientation = Qt::Vertical;
    bool showPlayerView = false;
    bool showCursorTile = true;
    bool showBorder = true;
    bool showGrid = false;
    bool showTilesetEditorMetatileGrid = false;
    bool showTilesetEditorLayerGrid = true;
    bool showTilesetEditorDivider = true;
    bool showTilesetEditorRawAttributes = false;
    bool showPaletteEditorUnusedColors = false;
    bool monitorFiles = true;
    bool tilesetCheckerboardFill = true;
    bool newMapHeaderSectionExpanded = false;
    NumberSystemMode tileNumberSystem = NumberSystemMode::Both;
    QString theme = QStringLiteral("default");
    QString wildMonChartTheme;
    QString textEditorOpenFolder;
    QString textEditorGotoLine;
    int paletteEditorBitDepth = 24;
    int projectSettingsTab = 0;
    ScriptAutocompleteMode scriptAutocompleteMode = ScriptAutocompleteMode::MapOnly;
    bool warpBehaviorWarningDisabled = false;
    bool eventDeleteWarningDisabled = false;
    bool eventOverlayEnabled = false;
    bool checkForUpdates = true;
    bool showProjectLoadingScreen = true;
    QDateTime lastUpdateCheckTime;
    QVersionNumber lastUpdateCheckVersion;
    QMap<QUrl, QDateTime> rateLimitTimes;
    QGraphicsPixmapItem::ShapeMode eventSelectionShapeMode = QGraphicsPixmapItem::MaskShape;
    bool shownInGameReloadMessage = false;
    GridSettings gridSettings;
    OrderedSet<LogType> statusBarLogTypes = { LogType::LOG_ERROR, LogType::LOG_WARN };
    QFont applicationFont;
    QFont mapListFont;
#ifdef Q_OS_MACOS
    // Since the release of the Retina display, Apple products use the Display P3 color space by default.
    // If we don't use this for exported images (which by default will either have no color space or the sRGB
    // color space) then they may appear to have different colors than the same image displayed in Porymap.
    std::optional<QColorSpace::NamedColorSpace> imageExportColorSpace = QColorSpace::DisplayP3;
#else
    // As of writing Qt has no way to get a reasonable color space from the user's environment,
    // so we export images without one and let them handle it.
    std::optional<QColorSpace::NamedColorSpace> imageExportColorSpace = {};
#endif
    QMap<QString,QString> trustedScriptHashes;

    FieldManager* getFieldManager() override {
        if (!m_fm) {
            m_fm = std::make_shared<FieldManager>();
            m_fm->addField(&this->reopenOnLaunch, "reopen_on_launch");
            m_fm->addField(&this->projectManuallyClosed, "project_manually_closed");
            m_fm->addField(&this->mapListTab, "map_list_tab", 0, 2);
            m_fm->addField(&this->mapListEditGroupsEnabled, "map_list_edit_groups_enabled");
            m_fm->addField(&this->mapListTabsHidingEmptyFolders, "map_list_tabs_hiding_empty_folders");
            m_fm->addField(&this->mapListLayoutsSorted, "map_list_layouts_sorted");
            m_fm->addField(&this->mapListLocationsSorted, "map_list_locations_sorted");
            m_fm->addField(&this->prettyCursors, "pretty_cursors");
            m_fm->addField(&this->mirrorConnectingMaps, "mirror_connecting_maps");
            m_fm->addField(&this->showDiveEmergeMaps, "show_dive_emerge_maps");
            m_fm->addField(&this->diveEmergeMapOpacity, "dive_emerge_map_opacity", 10, 90);
            m_fm->addField(&this->diveMapOpacity, "dive_map_opacity", 10, 90);
            m_fm->addField(&this->emergeMapOpacity, "emerge_map_opacity", 10, 90);
            m_fm->addField(&this->collisionOpacity, "collision_opacity", 0, 100);
            m_fm->addField(&this->collisionZoom, "collision_zoom", 10, 100);
            m_fm->addField(&this->metatilesZoom, "metatiles_zoom", 10, 100);
            m_fm->addField(&this->tilesetEditorMetatilesZoom, "tileset_editor_metatiles_zoom", 10, 100);
            m_fm->addField(&this->tilesetEditorTilesZoom, "tileset_editor_tiles_zoom", 10, 100);
            m_fm->addField(&this->tilesetEditorLayerOrientation, "tileset_editor_layer_orientation");
            m_fm->addField(&this->showPlayerView, "show_player_view");
            m_fm->addField(&this->showCursorTile, "show_cursor_tile");
            m_fm->addField(&this->showBorder, "show_border");
            m_fm->addField(&this->showGrid, "show_grid");
            m_fm->addField(&this->showTilesetEditorMetatileGrid, "show_tileset_editor_metatile_grid");
            m_fm->addField(&this->showTilesetEditorLayerGrid, "show_tileset_editor_layer_grid");
            m_fm->addField(&this->showTilesetEditorDivider, "show_tileset_editor_divider");
            m_fm->addField(&this->showTilesetEditorRawAttributes, "show_tileset_editor_raw_attributes");
            m_fm->addField(&this->showPaletteEditorUnusedColors, "show_palette_editor_unused_colors");
            m_fm->addField(&this->monitorFiles, "monitor_files");
            m_fm->addField(&this->tilesetCheckerboardFill, "tileset_checkerboard_fill");
            m_fm->addField(&this->newMapHeaderSectionExpanded, "new_map_header_section_expanded");
            m_fm->addField(&this->tileNumberSystem, "tile_number_system");
            m_fm->addField(&this->theme, "theme");
            m_fm->addField(&this->wildMonChartTheme, "wild_mon_chart_theme");
            m_fm->addField(&this->textEditorOpenFolder, "text_editor_open_folder");
            m_fm->addField(&this->textEditorGotoLine, "text_editor_goto_line");
            m_fm->addField(&this->paletteEditorBitDepth, "palette_editor_bit_depth", {24,15});
            m_fm->addField(&this->projectSettingsTab, "project_settings_tab");
            m_fm->addField(&this->scriptAutocompleteMode, "script_autocomplete_mode");
            m_fm->addField(&this->warpBehaviorWarningDisabled, "warp_behavior_warning_disabled");
            m_fm->addField(&this->eventDeleteWarningDisabled, "event_delete_warning_disabled");
            m_fm->addField(&this->eventOverlayEnabled, "event_overlay_enabled");
            m_fm->addField(&this->checkForUpdates, "check_for_updates");
            m_fm->addField(&this->showProjectLoadingScreen, "show_project_loading_screen");
            m_fm->addField(&this->lastUpdateCheckTime, "last_update_check_time");
            m_fm->addField(&this->lastUpdateCheckVersion, "last_update_check_version");
            m_fm->addField(&this->rateLimitTimes, "rate_limit_times");
            m_fm->addField(&this->eventSelectionShapeMode, "event_selection_shape_mode");
            m_fm->addField(&this->shownInGameReloadMessage, "shown_in_game_reload_message");
            m_fm->addField(&this->gridSettings, "map_grid");
            m_fm->addField(&this->statusBarLogTypes, "status_bar_log_types");
            m_fm->addField(&this->applicationFont, "application_font");
            m_fm->addField(&this->mapListFont, "map_list_font");
            m_fm->addField(&this->imageExportColorSpace, "image_export_color_space");
            m_fm->addField(&this->trustedScriptHashes, "trusted_script_hashes");

            m_fm->addField(&this->recentProjects, "recent_projects");
            m_fm->addField(&this->savedGeometryMap, "geometry");
            m_fm->addField(&this->geometryVersion, "geometry_version");
        }
        return m_fm.get();
    };

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;

private:
    std::shared_ptr<FieldManager> m_fm = nullptr;
    QStringList recentProjects;
    QMap<QString, QByteArray> savedGeometryMap;
    int geometryVersion = 0;
};

extern PorymapConfig porymapConfig;

#endif // PORYMAPCONFIG_H
