#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QObject>
#include <QByteArrayList>
#include <QSize>
#include <QKeySequence>
#include <QMultiMap>
#include <QDateTime>
#include <QUrl>
#include <QVersionNumber>
#include <QGraphicsPixmapItem>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QColorSpace>
#include <set>

#include "converter.h"
#include "events.h"
#include "gridsettings.h"
#include "scriptsettings.h"
#include "log.h"
#include "basegame.h"
#include "orderedset.h"
#include "block.h"

// TODO: Go through and re-test the default window geometries.
// TODO: Value validation? Make sure anything no longer validated can't cause problems.
// TODO: Documentation
// TODO: File splits

extern const QVersionNumber porymapVersion;

// Distance in pixels from the edge of a GBA screen (240x160) to the center 16x16 pixels.
#define GBA_H_DIST_TO_CENTER ((240-16)/2)
#define GBA_V_DIST_TO_CENTER ((160-16)/2)

enum ScriptAutocompleteMode {
    MapOnly,
    MapAndCommon,
    All,
};

class KeyValueConfigBase
{
public:
    explicit KeyValueConfigBase(const QString& filename)
        : m_root(QString()),
          m_filename(filename),
          m_filepath(filename)
    { };
    virtual ~KeyValueConfigBase() {};

    // Writes the contents of the config to disk.
    // Returns true if saving was successful, false otherwise.
    virtual bool save();

    // Loads the contents of the config from disk.
    // The file to load from depends on the file name given to
    // the constructor, and the root given to setRoot, if any.
    // A successful load includes initializing an empty or non-existing file.
    virtual bool load();

    virtual QJsonObject toJson() const;
    virtual void loadFromJson(const QJsonObject& obj);

    void setRoot(const QString& dir);
    QString root() const { return m_root; }
    QString filepath() const { return m_filepath; }
    QString filename() const { return m_filename; }
protected:
    virtual void initializeFromEmpty() {};
    virtual QJsonObject getDefaultJson() const { return QJsonObject(); }

    struct FieldManager {
        QJsonValue (*get)(const KeyValueConfigBase*);
        QStringList (*set)(KeyValueConfigBase*, const QJsonValue&);
    };
    template <typename ConfigType, auto Member>
    static FieldManager makeFieldManager() {
        // Deduce the type from the pointer to the member variable.
        using T = std::remove_reference_t<decltype(std::declval<ConfigType>().*Member)>;
        return FieldManager{
            .get = [](const KeyValueConfigBase* base) -> QJsonValue {
                auto c = static_cast<const ConfigType*>(base);
                return Converter<T>::toJson(c->*Member);
            },
            .set = [](KeyValueConfigBase* base, const QJsonValue& json) {
                QStringList errors;
                const T value = Converter<T>::fromJson(json, &errors);
                if (errors.isEmpty()) static_cast<ConfigType*>(base)->*Member = value;
                return errors;
            }
        };
    }
    virtual const QHash<QString,FieldManager>& registeredFields() const {
        static QHash<QString,FieldManager> empty;
        return empty;
    }

    virtual bool parseJsonKeyValue(const QString& key, const QJsonValue& value);
    virtual bool parseLegacyKeyValue(const QString& , const QString& ) {return false;}

    QString m_root;
    QString m_filename;
    QString m_filepath;
private:
    bool loadLegacy();

    // TODO: Make this setting accessible somewhere
    bool m_saveAllFields = true;
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
    bool displayIdsHexadecimal = true;
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
    int imageExportColorSpaceId = static_cast<int>(QColorSpace::DisplayP3);
#else
    // As of writing Qt has no way to get a reasonable color space from the user's environment,
    // so we export images without one and let them handle it.
    int imageExportColorSpaceId = 0;
#endif
    QMap<QString,QString> trustedScriptHashes;

#define REGISTER(key, member) {QStringLiteral(key), makeFieldManager<PorymapConfig, &PorymapConfig::member>()}
    const QHash<QString,FieldManager>& registeredFields() const override {
        static const QHash<QString,FieldManager> fields = {
            REGISTER("reopen_on_launch", reopenOnLaunch),
            REGISTER("project_manually_closed", projectManuallyClosed),
            REGISTER("map_list_tab", mapListTab),
            REGISTER("map_list_edit_groups_enabled", mapListEditGroupsEnabled),
            REGISTER("map_list_tabs_hiding_empty_folders", mapListTabsHidingEmptyFolders),
            REGISTER("map_list_layouts_sorted", mapListLayoutsSorted),
            REGISTER("map_list_locations_sorted", mapListLocationsSorted),
            REGISTER("pretty_cursors", prettyCursors),
            REGISTER("mirror_connecting_maps", mirrorConnectingMaps),
            REGISTER("show_dive_emerge_maps", showDiveEmergeMaps),
            REGISTER("dive_emerge_map_opacity", diveEmergeMapOpacity),
            REGISTER("dive_map_opacity", diveMapOpacity),
            REGISTER("emerge_map_opacity", emergeMapOpacity),
            REGISTER("collision_opacity", collisionOpacity),
            REGISTER("collision_zoom", collisionZoom),
            REGISTER("metatiles_zoom", metatilesZoom),
            REGISTER("tileset_editor_metatiles_zoom", tilesetEditorMetatilesZoom),
            REGISTER("tileset_editor_tiles_zoom", tilesetEditorTilesZoom),
            REGISTER("tileset_editor_layer_orientation", tilesetEditorLayerOrientation),
            REGISTER("show_player_view", showPlayerView),
            REGISTER("show_cursor_tile", showCursorTile),
            REGISTER("show_border", showBorder),
            REGISTER("show_grid", showGrid),
            REGISTER("show_tileset_editor_metatile_grid", showTilesetEditorMetatileGrid),
            REGISTER("show_tileset_editor_layer_grid", showTilesetEditorLayerGrid),
            REGISTER("show_tileset_editor_divider", showTilesetEditorDivider),
            REGISTER("show_tileset_editor_raw_attributes", showTilesetEditorRawAttributes),
            REGISTER("show_palette_editor_unused_colors", showPaletteEditorUnusedColors),
            REGISTER("monitor_files", monitorFiles),
            REGISTER("tileset_checkerboard_fill", tilesetCheckerboardFill),
            REGISTER("new_map_header_section_expanded", newMapHeaderSectionExpanded),
            REGISTER("display_ids_hexadecimal", displayIdsHexadecimal),
            REGISTER("theme", theme),
            REGISTER("wild_mon_chart_theme", wildMonChartTheme),
            REGISTER("text_editor_open_folder", textEditorOpenFolder),
            REGISTER("text_editor_goto_line", textEditorGotoLine),
            REGISTER("palette_editor_bit_depth", paletteEditorBitDepth),
            REGISTER("project_settings_tab", projectSettingsTab),
            REGISTER("script_autocomplete_mode", scriptAutocompleteMode),
            REGISTER("warp_behavior_warning_disabled", warpBehaviorWarningDisabled),
            REGISTER("event_delete_warning_disabled", eventDeleteWarningDisabled),
            REGISTER("event_overlay_enabled", eventOverlayEnabled),
            REGISTER("check_for_updates", checkForUpdates),
            REGISTER("show_project_loading_screen", showProjectLoadingScreen),
            REGISTER("last_update_check_time", lastUpdateCheckTime),
            REGISTER("last_update_check_version", lastUpdateCheckVersion),
            REGISTER("rate_limit_times", rateLimitTimes),
            REGISTER("event_selection_shape_mode", eventSelectionShapeMode),
            REGISTER("shown_in_game_reload_message", shownInGameReloadMessage),
            REGISTER("map_grid", gridSettings),
            REGISTER("status_bar_log_types", statusBarLogTypes),
            REGISTER("application_font", applicationFont),
            REGISTER("map_list_font", mapListFont),
            REGISTER("image_export_color_space_id", imageExportColorSpaceId),
            REGISTER("trusted_script_hashes", trustedScriptHashes),

            REGISTER("recent_projects", recentProjects),
            REGISTER("geometry", savedGeometryMap),
            REGISTER("geometry_version", geometryVersion),
        };
        return fields;
    };
#undef REGISTER

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;

private:
    QStringList recentProjects;
    QMap<QString, QByteArray> savedGeometryMap;
    int geometryVersion = 0;
};

extern PorymapConfig porymapConfig;

enum ProjectIdentifier {
    symbol_facing_directions,
    symbol_obj_event_gfx_pointers,
    symbol_pokemon_icon_table,
    symbol_attribute_table,
    symbol_tilesets_prefix,
    symbol_dynamic_map_name,
    define_obj_event_count,
    define_min_level,
    define_max_level,
    define_max_encounter_rate,
    define_tiles_primary,
    define_tiles_total,
    define_metatiles_primary,
    define_pals_primary,
    define_pals_total,
    define_tiles_per_metatile,
    define_map_size,
    define_map_offset_width,
    define_map_offset_height,
    define_mask_metatile,
    define_mask_collision,
    define_mask_elevation,
    define_mask_behavior,
    define_mask_layer,
    define_attribute_behavior,
    define_attribute_layer,
    define_attribute_terrain,
    define_attribute_encounter,
    define_metatile_label_prefix,
    define_heal_locations_prefix,
    define_layout_prefix,
    define_map_prefix,
    define_map_dynamic,
    define_map_empty,
    define_map_section_prefix,
    define_map_section_empty,
    define_species_prefix,
    define_species_empty,
    regex_behaviors,
    regex_obj_event_gfx,
    regex_items,
    regex_flags,
    regex_vars,
    regex_movement_types,
    regex_map_types,
    regex_battle_scenes,
    regex_weather,
    regex_coord_event_weather,
    regex_secret_bases,
    regex_sign_facing_directions,
    regex_trainer_types,
    regex_music,
    regex_encounter_types,
    regex_terrain_types,
    pals_output_extension,
    tiles_output_extension,
};

enum ProjectFilePath {
    data_map_folders,
    data_scripts_folders,
    data_layouts_folders,
    data_primary_tilesets_folders,
    data_secondary_tilesets_folders,
    data_event_scripts,
    json_map_groups,
    json_layouts,
    json_wild_encounters,
    json_heal_locations,
    json_region_map_entries,
    json_region_porymap_cfg,
    tilesets_headers,
    tilesets_graphics,
    tilesets_metatiles,
    tilesets_headers_asm,
    tilesets_graphics_asm,
    tilesets_metatiles_asm,
    data_obj_event_gfx_pointers,
    data_obj_event_gfx_info,
    data_obj_event_pic_tables,
    data_obj_event_gfx,
    data_pokemon_gfx,
    constants_global,
    constants_items,
    constants_flags,
    constants_vars,
    constants_weather,
    constants_songs,
    constants_pokemon,
    constants_map_types,
    constants_trainer_types,
    constants_secret_bases,
    constants_obj_event_movement,
    constants_obj_events,
    constants_event_bg,
    constants_metatile_labels,
    constants_metatile_behaviors,
    constants_species,
    constants_fieldmap,
    global_fieldmap,
    fieldmap,
    initial_facing_table,
    wild_encounter,
    pokemon_icon_table,
    pokemon_gfx,
};

class ProjectConfig: public KeyValueConfigBase
{
public:
    ProjectConfig(const QString& root = QString()) : KeyValueConfigBase(QStringLiteral("porymap.project.json")) {
        setRoot(root);
    }
    ProjectConfig(BaseGame::Version version, const QString& root = QString()) : ProjectConfig(root) {
        setVersionSpecificDefaults(version);
    }

    virtual bool save() override;

    virtual void loadFromJson(const QJsonObject& obj) override;

    QString projectDir() const { return m_root; } // Alias for root()
    void setVersionSpecificDefaults(BaseGame::Version baseGameVersion);

    void setFilePath(ProjectFilePath pathId, const QString& path);
    void setFilePath(const QString& pathId, const QString& path);
    QString getCustomFilePath(ProjectFilePath pathId);
    QString getCustomFilePath(const QString& pathId);
    QString getFilePath(ProjectFilePath pathId);

    void setIdentifier(ProjectIdentifier id, const QString& text);
    void setIdentifier(const QString& id, const QString& text);
    QString getCustomIdentifier(ProjectIdentifier id);
    QString getCustomIdentifier(const QString& id);
    QString getIdentifier(ProjectIdentifier id);

    static const QMap<ProjectIdentifier, QPair<QString, QString>> defaultIdentifiers;
    static const QMap<ProjectFilePath, QPair<QString, QString>> defaultPaths;

    BaseGame::Version baseGameVersion = BaseGame::Version::none;
    bool usePoryScript = false;
    bool useCustomBorderSize = false;
    bool eventWeatherTriggerEnabled = false;
    bool eventSecretBaseEnabled = false;
    bool hiddenItemQuantityEnabled = false;
    bool hiddenItemRequiresItemfinderEnabled = false;
    bool healLocationRespawnDataEnabled = false;
    bool eventCloneObjectEnabled = false;
    bool floorNumberEnabled = false;
    bool createMapTextFileEnabled = false;
    bool tripleLayerMetatilesEnabled = false;
    uint16_t defaultMetatileId = 1;
    uint16_t defaultElevation = 3;
    uint16_t defaultCollision = 0;
    QSize defaultMapSize = QSize(20,20);
    QList<uint16_t> newMapBorderMetatileIds;
    QString defaultPrimaryTileset = QStringLiteral("gTileset_General");
    QString defaultSecondaryTileset;
    bool tilesetsHaveCallback = true;
    bool tilesetsHaveIsCompressed = true;
    QColor transparencyColor = QColorConstants::Black;
    bool preserveMatchingOnlyData = false;
    int metatileAttributesSize = 2;
    uint32_t metatileBehaviorMask = 0;
    uint32_t metatileTerrainTypeMask = 0;
    uint32_t metatileEncounterTypeMask = 0;
    uint32_t metatileLayerTypeMask = 0;
    uint16_t blockMetatileIdMask = Block::DefaultMetatileIdMask;
    uint16_t blockCollisionMask = Block::DefaultCollisionMask;
    uint16_t blockElevationMask = Block::DefaultElevationMask;
    uint16_t unusedTileNormal = 0x3014;
    uint16_t unusedTileCovered = 0x0000;
    uint16_t unusedTileSplit = 0x0000;
    bool mapAllowFlagsEnabled = true;
    QString eventsTabIconPath;
    QString collisionSheetPath;
    QSize collisionSheetSize = QSize(2, 16);
    QMargins playerViewDistance = QMargins(GBA_H_DIST_TO_CENTER, GBA_V_DIST_TO_CENTER, GBA_H_DIST_TO_CENTER, GBA_V_DIST_TO_CENTER);
    OrderedSet<uint32_t> warpBehaviors;
    int maxEventsPerGroup = 255;
    int metatileSelectorWidth = 8;
    QStringList globalConstantsFilepaths;
    QMap<QString,QString> globalConstants;
    QList<ScriptSettings> customScripts;
    QMap<Event::Group, QString> eventIconPaths;
    QMap<QString, QString> pokemonIconPaths;
    QVersionNumber minimumVersion;

#define REGISTER(key, member) {QStringLiteral(key), makeFieldManager<ProjectConfig, &ProjectConfig::member>()}
    const QHash<QString,FieldManager>& registeredFields() const override {
        static const QHash<QString,FieldManager> fields = {
            REGISTER("base_game_version", baseGameVersion),
            REGISTER("use_poryscript", usePoryScript),
            REGISTER("use_custom_border_size", useCustomBorderSize),
            REGISTER("enable_event_weather_trigger", eventWeatherTriggerEnabled),
            REGISTER("enable_event_secret_base", eventSecretBaseEnabled),
            REGISTER("enable_hidden_item_quantity", hiddenItemQuantityEnabled),
            REGISTER("enable_hidden_item_requires_itemfinder", hiddenItemRequiresItemfinderEnabled),
            REGISTER("enable_heal_location_respawn_data", healLocationRespawnDataEnabled),
            REGISTER("enable_event_clone_object", eventCloneObjectEnabled),
            REGISTER("enable_floor_number", floorNumberEnabled),
            REGISTER("create_map_text_file", createMapTextFileEnabled),
            REGISTER("enable_triple_layer_metatiles", tripleLayerMetatilesEnabled),
            REGISTER("default_metatile_id", defaultMetatileId),
            REGISTER("default_elevation", defaultElevation),
            REGISTER("default_collision", defaultCollision),
            REGISTER("default_map_size", defaultMapSize),
            REGISTER("new_map_border_metatiles", newMapBorderMetatileIds),
            REGISTER("default_primary_tileset", defaultPrimaryTileset),
            REGISTER("default_secondary_tileset", defaultSecondaryTileset),
            REGISTER("tilesets_have_callback", tilesetsHaveCallback),
            REGISTER("tilesets_have_is_compressed", tilesetsHaveIsCompressed),
            REGISTER("transparency_color", transparencyColor),
            REGISTER("preserve_matching_only_data", preserveMatchingOnlyData),
            REGISTER("metatile_attributes_size", metatileAttributesSize),
            REGISTER("metatile_behavior_mask", metatileBehaviorMask),
            REGISTER("metatile_terrain_type_mask", metatileTerrainTypeMask),
            REGISTER("metatile_encounter_type_mask", metatileEncounterTypeMask),
            REGISTER("metatile_layer_type_mask", metatileLayerTypeMask),
            REGISTER("block_metatile_id_mask", blockMetatileIdMask),
            REGISTER("block_collision_mask", blockCollisionMask),
            REGISTER("block_elevation_mask", blockElevationMask),
            REGISTER("unused_tile_normal", unusedTileNormal),
            REGISTER("unused_tile_covered", unusedTileCovered),
            REGISTER("unused_tile_split", unusedTileSplit),
            REGISTER("enable_map_allow_flags", mapAllowFlagsEnabled),
            REGISTER("events_tab_icon_path", eventsTabIconPath),
            REGISTER("collision_sheet_path", collisionSheetPath),
            REGISTER("collision_sheet_size", collisionSheetSize),
            REGISTER("player_view_distance", playerViewDistance),
            REGISTER("warp_behaviors", warpBehaviors),
            REGISTER("max_events_per_group", maxEventsPerGroup),
            REGISTER("metatile_selector_width", metatileSelectorWidth),
            REGISTER("global_constants_filepaths", globalConstantsFilepaths),
            REGISTER("global_constants", globalConstants),
            REGISTER("custom_scripts", customScripts),
            REGISTER("event_icon_paths", eventIconPaths),
            REGISTER("pokemon_icon_paths", pokemonIconPaths),
            REGISTER("minimum_version", minimumVersion),

            REGISTER("custom_identifiers", identifiers),
            REGISTER("custom_file_paths", filePaths),
        };
        return fields;
    }
#undef REGISTER

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;
    virtual void initializeFromEmpty() override;

private:
    ProjectFilePath reverseDefaultPaths(const QString& str);
    ProjectIdentifier reverseDefaultIdentifier(const QString& str);

    QMap<ProjectIdentifier, QString> identifiers;
    QMap<ProjectFilePath, QString> filePaths;
};

extern ProjectConfig projectConfig;

class UserConfig: public KeyValueConfigBase
{
public:
    UserConfig(const QString& root = QString()) : KeyValueConfigBase(QStringLiteral("porymap.user.json")) {
        setRoot(root);
    }

    virtual void loadFromJson(const QJsonObject& obj) override;

    QString projectDir() const { return m_root; } // Alias for root()

    QString recentMapOrLayout;
    QString prefabsFilepath;
    bool prefabsImportPrompted = false;
    bool useEncounterJson = true;
    QList<ScriptSettings> customScripts;

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;

#define REGISTER(key, member) {QStringLiteral(key), makeFieldManager<UserConfig, &UserConfig::member>()}
    const QHash<QString,FieldManager>& registeredFields() const override {
        static const QHash<QString,FieldManager> fields = {
            REGISTER("recent_map_or_layout", recentMapOrLayout),
            REGISTER("prefabs_filepath", prefabsFilepath),
            REGISTER("prefabs_import_prompted", prefabsImportPrompted),
            REGISTER("use_encounter_json", useEncounterJson),
            REGISTER("custom_scripts", customScripts),
        };
        return fields;
    }
#undef REGISTER
};

extern UserConfig userConfig;

class QAction;
class Shortcut;

class ShortcutsConfig : public KeyValueConfigBase
{
public:
    ShortcutsConfig() : KeyValueConfigBase(QStringLiteral("shortcuts.json")) {
        setRoot(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    virtual QJsonObject toJson() const override;
    virtual void loadFromJson(const QJsonObject& obj) override;

    // Call this before applying user shortcuts so that the user can restore defaults.
    void setDefaultShortcuts(const QObjectList& objects);
    QList<QKeySequence> defaultShortcuts(const QObject *object) const;

    void setUserShortcuts(const QObjectList& objects);
    void setUserShortcuts(const QMultiMap<const QObject *, QKeySequence>& objects_keySequences);
    QList<QKeySequence> userShortcuts(const QObject *object) const;

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;

private:
    QMultiMap<QString, QKeySequence> user_shortcuts;
    QMultiMap<QString, QKeySequence> default_shortcuts;

    enum StoreType {
        User,
        Default
    };

    QString cfgKey(const QObject *object) const;
    QList<QKeySequence> currentShortcuts(const QObject *object) const;

    void storeShortcutsFromList(StoreType storeType, const QObjectList& objects);
    void storeShortcuts(
            StoreType storeType,
            const QString& cfgKey,
            const QList<QKeySequence>& keySequences);
};

extern ShortcutsConfig shortcutsConfig;

#endif // CONFIG_H
