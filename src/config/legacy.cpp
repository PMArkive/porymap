#include "config.h"
#include "log.h"
#include "tile.h"
#include "block.h"

#include <QString>
#include <QDir>
#include <QColor>


QString getLegacyFilename(const QString &newFilename) {
    static const QMap<QString, QString> map = {
        {QStringLiteral("settings.json"),        QStringLiteral("porymap.cfg")},
        {QStringLiteral("shortcuts.json"),       QStringLiteral("porymap.shortcuts.cfg")},
        {QStringLiteral("porymap.project.json"), QStringLiteral("porymap.project.cfg")},
        {QStringLiteral("porymap.user.json"),    QStringLiteral("porymap.user.cfg")},
    };
    return map.value(newFilename);
}

bool KeyValueConfigBase::loadLegacy() {
    const QString oldFilename = getLegacyFilename(filename());
    if (oldFilename.isEmpty()) return false;

    QDir dir(root());
    QFile file(dir.absoluteFilePath(oldFilename));
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return false;

    QTextStream in(&file);
    static const QRegularExpression re("^(?<key>[^=]+)=(?<value>.*)$");
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        int commentIndex = line.indexOf("#");
        if (commentIndex >= 0) {
            line = line.left(commentIndex).trimmed();
        }

        if (line.length() == 0) {
            continue;
        }

        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            parseLegacyKeyValue(match.captured("key").trimmed(), match.captured("value").trimmed());
        }
    }
    logInfo(QString("Loaded legacy config file '%1'").arg(oldFilename));

    // Save before deleting the old config file to ensure no data is lost.
    if (save()) {
        if (!file.remove()) logWarn(QString("Failed to delete legacy config file '%1'.").arg(oldFilename));
        else logInfo(QString("Deleted legacy config file '%1'.").arg(oldFilename));
    }
    return true;
}

bool toBool(const QString &value) {
    return (value.toInt(nullptr, 0) != 0);
}

int toInt(const QString &value, int min = INT_MIN, int max = INT_MAX, int defaultValue = 0) {
    bool ok;
    int result = value.toInt(&ok, 0);
    if (!ok) result = defaultValue;
    return qBound(min, result, max);
}

uint32_t toUInt(const QString &value, uint32_t min = 0, uint32_t max = UINT_MAX, uint32_t defaultValue = 0) {
    bool ok;
    uint32_t result = value.toUInt(&ok, 0);
    if (!ok) result = defaultValue;
    return qBound(min, result, max);
}

QColor toColor(const QString &value, const QColor &defaultValue = QColor(Qt::black)) {
    if (value.isEmpty()) return QColor();
    QColor color = QColor("#" + value);
    if (!color.isValid()) color = defaultValue;
    return color;
}

bool PorymapConfig::parseLegacyKeyValue(const QString &key, const QString &value) {
    if (key == "recent_project") {
        this->recentProjects = value.split(",", Qt::SkipEmptyParts);
        this->recentProjects.removeDuplicates();
    } else if (key == "project_manually_closed") {
        this->projectManuallyClosed = toBool(value);
    } else if (key == "reopen_on_launch") {
        this->reopenOnLaunch = toBool(value);
    } else if (key == "pretty_cursors") {
        this->prettyCursors = toBool(value);
    } else if (key == "map_list_tab") {
        this->mapListTab = toInt(value, 0, 2, 0);
    } else if (key == "map_list_edit_groups_enabled") {
        this->mapListEditGroupsEnabled = toBool(value);
    } else if (key.startsWith("map_list_hide_empty_enabled/")) {
        bool ok;
        int tab = key.mid(QStringLiteral("map_list_hide_empty_enabled/").length()).toInt(&ok, 0);
        if (ok && toBool(value)) this->mapListTabsHidingEmptyFolders.insert(tab);
    } else if (key == "map_list_layouts_sorted") {
        this->mapListLayoutsSorted = toBool(value);
    } else if (key == "map_list_locations_sorted") {
        this->mapListLocationsSorted = toBool(value);
    } else if (key == "mirror_connecting_maps") {
        this->mirrorConnectingMaps = toBool(value);
    } else if (key == "show_dive_emerge_maps") {
        this->showDiveEmergeMaps = toBool(value);
    } else if (key == "dive_emerge_map_opacity") {
        this->diveEmergeMapOpacity = toInt(value, 10, 90, 30);
    } else if (key == "dive_map_opacity") {
        this->diveMapOpacity = toInt(value, 10, 90, 15);
    } else if (key == "emerge_map_opacity") {
        this->emergeMapOpacity = toInt(value, 10, 90, 15);
    } else if (key == "collision_opacity") {
        this->collisionOpacity = toInt(value, 0, 100, 50);
    } else if (key == "metatiles_zoom") {
        this->metatilesZoom = toInt(value, 10, 100, 30);
    } else if (key == "collision_zoom") {
        this->collisionZoom = toInt(value, 10, 100, 30);
    } else if (key == "tileset_editor_metatiles_zoom") {
        this->tilesetEditorMetatilesZoom = toInt(value, 10, 100, 30);
    } else if (key == "tileset_editor_tiles_zoom") {
        this->tilesetEditorTilesZoom = toInt(value, 10, 100, 30);
     } else if (key == "tileset_editor_layer_orientation") {
        // Being explicit here to avoid casting out-of-range values.
        this->tilesetEditorLayerOrientation = (toInt(value) == static_cast<int>(Qt::Horizontal)) ? Qt::Horizontal : Qt::Vertical;
    } else if (key == "show_player_view") {
        this->showPlayerView = toBool(value);
    } else if (key == "show_cursor_tile") {
        this->showCursorTile = toBool(value);
    } else if (key == "show_border") {
        this->showBorder = toBool(value);
    } else if (key == "show_grid") {
        this->showGrid = toBool(value);
    } else if (key == "show_tileset_editor_metatile_grid") {
        this->showTilesetEditorMetatileGrid = toBool(value);
    } else if (key == "show_tileset_editor_layer_grid") {
        this->showTilesetEditorLayerGrid = toBool(value);
    } else if (key == "show_tileset_editor_divider") {
        this->showTilesetEditorDivider = toBool(value);
    } else if (key == "show_tileset_editor_raw_attributes") {
        this->showTilesetEditorRawAttributes = toBool(value);
    } else if (key == "show_palette_editor_unused_colors") {
        this->showPaletteEditorUnusedColors = toBool(value);
    } else if (key == "monitor_files") {
        this->monitorFiles = toBool(value);
    } else if (key == "tileset_checkerboard_fill") {
        this->tilesetCheckerboardFill = toBool(value);
    } else if (key == "new_map_header_section_expanded") {
        this->newMapHeaderSectionExpanded = toBool(value);
    } else if (key == "theme") {
        this->theme = value;
    } else if (key == "wild_mon_chart_theme") {
        this->wildMonChartTheme = value;
    } else if (key == "text_editor_open_directory") {
        this->textEditorOpenFolder = value;
    } else if (key == "text_editor_goto_line") {
        this->textEditorGotoLine = value;
    } else if (key == "palette_editor_bit_depth") {
        int bitDepth = toInt(value, 15, 24, 24);
        if (bitDepth == 15 || bitDepth == 24){
            this->paletteEditorBitDepth = bitDepth;
        }
    } else if (key == "project_settings_tab") {
        this->projectSettingsTab = toInt(value, 0);
    } else if (key == "load_all_event_scripts") { // Old setting replaced by script_autocomplete_mode
        this->scriptAutocompleteMode = toBool(value) ? ScriptAutocompleteMode::All : ScriptAutocompleteMode::MapOnly;
    } else if (key == "script_autocomplete_mode") {
        this->scriptAutocompleteMode = static_cast<ScriptAutocompleteMode>(toInt(value, ScriptAutocompleteMode::MapOnly, ScriptAutocompleteMode::All));
    } else if (key == "warp_behavior_warning_disabled") {
        this->warpBehaviorWarningDisabled = toBool(value);
    } else if (key == "event_delete_warning_disabled") {
        this->eventDeleteWarningDisabled = toBool(value);
    } else if (key == "event_overlay_enabled") {
        this->eventOverlayEnabled = toBool(value);
    } else if (key == "check_for_updates") {
        this->checkForUpdates = toBool(value);
    } else if (key == "show_project_loading_screen") {
        this->showProjectLoadingScreen = toBool(value);
    } else if (key == "last_update_check_time") {
        this->lastUpdateCheckTime = QDateTime::fromString(value).toLocalTime();
    } else if (key == "last_update_check_version") {
        auto version = QVersionNumber::fromString(value);
        if (version.segmentCount() == 3) {
            this->lastUpdateCheckVersion = version;
        }
    } else if (key.startsWith("rate_limit_time/")) {
        static const QRegularExpression regex("\\brate_limit_time/(?<url>.+)");
        QRegularExpressionMatch match = regex.match(key);
        if (match.hasMatch()) {
            this->rateLimitTimes.insert(match.captured("url"), QDateTime::fromString(value).toLocalTime());
        }
    } else if (key == "event_selection_shape_mode") {
        if (value == "mask") {
            this->eventSelectionShapeMode = QGraphicsPixmapItem::MaskShape;
        } else if (value == "bounding_rect") {
            this->eventSelectionShapeMode = QGraphicsPixmapItem::BoundingRectShape;
        }
    } else if (key == "shown_in_game_reload_message") {
        this->shownInGameReloadMessage = toBool(value);
    } else if (key == "grid_width") {
        this->gridSettings.width = toUInt(value);
    } else if (key == "grid_height") {
        this->gridSettings.height = toUInt(value);
    } else if (key == "grid_x") {
        this->gridSettings.offsetX = toInt(value, 0, 999);
    } else if (key == "grid_y") {
        this->gridSettings.offsetY = toInt(value, 0, 999);
    } else if (key == "grid_style") {
        this->gridSettings.style = GridSettings::getStyleFromName(value);
    } else if (key == "grid_color") {
        this->gridSettings.color = toColor(value);
    } else if (key == "status_bar_log_types") {
        this->statusBarLogTypes.clear();
        auto typeStrings = value.split(",", Qt::SkipEmptyParts);
        for (const auto &typeString : typeStrings) {
            LogType type = static_cast<LogType>(toInt(typeString, 0, 2));
            this->statusBarLogTypes.insert(type);
        }
    } else if (key == "application_font") {
        this->applicationFont = QFont();
        this->applicationFont.fromString(value);
    } else if (key == "map_list_font") {
        this->mapListFont = QFont();
        this->mapListFont.fromString(value);
    } else {
        return false;
    }
    return true;
}

bool ProjectConfig::parseLegacyKeyValue(const QString &key, const QString &value_) {
    QString value(value_);
    if (key == "base_game_version") {
        this->baseGameVersion = BaseGame::stringToVersion(value);
    } else if (key == "use_poryscript") {
        this->usePoryScript = toBool(value);
    } else if (key == "use_custom_border_size") {
        this->useCustomBorderSize = toBool(value);
    } else if (key == "enable_event_weather_trigger") {
        this->eventWeatherTriggerEnabled = toBool(value);
    } else if (key == "enable_event_secret_base") {
        this->eventSecretBaseEnabled = toBool(value);
    } else if (key == "enable_hidden_item_quantity") {
        this->hiddenItemQuantityEnabled = toBool(value);
    } else if (key == "enable_hidden_item_requires_itemfinder") {
        this->hiddenItemRequiresItemfinderEnabled = toBool(value);
    } else if (key == "enable_heal_location_respawn_data") {
        this->healLocationRespawnDataEnabled = toBool(value);
    } else if (key == "enable_event_clone_object" || key == "enable_object_event_in_connection") {
        this->eventCloneObjectEnabled = toBool(value);
    } else if (key == "enable_floor_number") {
        this->floorNumberEnabled = toBool(value);
    } else if (key == "create_map_text_file") {
        this->createMapTextFileEnabled = toBool(value);
    } else if (key == "enable_triple_layer_metatiles") {
        this->tripleLayerMetatilesEnabled = toBool(value);
    } else if (key == "default_metatile") {
        this->defaultMetatileId = toUInt(value, 0, Block::MaxValue);
    } else if (key == "default_elevation") {
        this->defaultElevation = toUInt(value, 0, Block::MaxValue);
    } else if (key == "default_collision") {
        this->defaultCollision = toUInt(value, 0, Block::MaxValue);
    } else if (key == "default_map_width") {
        this->defaultMapSize.setWidth(toInt(value, 1));
    } else if (key == "default_map_height") {
        this->defaultMapSize.setHeight(toInt(value, 1));
    } else if (key == "new_map_border_metatiles") {
        this->newMapBorderMetatileIds.clear();
        QList<QString> metatileIds = value.split(",");
        for (int i = 0; i < metatileIds.size(); i++) {
            int metatileId = toUInt(metatileIds.at(i), 0, Block::MaxValue);
            this->newMapBorderMetatileIds.append(metatileId);
        }
    } else if (key == "default_primary_tileset") {
        this->defaultPrimaryTileset = value;
    } else if (key == "default_secondary_tileset") {
        this->defaultSecondaryTileset = value;
    } else if (key == "metatile_attributes_size") {
        int size = toInt(value, 1, 4, 2);
        if (!(size & (size - 1))) this->metatileAttributesSize = size;
    } else if (key == "metatile_behavior_mask") {
        this->metatileBehaviorMask = toUInt(value);
    } else if (key == "metatile_terrain_type_mask") {
        this->metatileTerrainTypeMask = toUInt(value);
    } else if (key == "metatile_encounter_type_mask") {
        this->metatileEncounterTypeMask = toUInt(value);
    } else if (key == "metatile_layer_type_mask") {
        this->metatileLayerTypeMask = toUInt(value);
    } else if (key == "block_metatile_id_mask") {
        this->blockMetatileIdMask = toUInt(value, 0, Block::MaxValue);
    } else if (key == "block_collision_mask") {
        this->blockCollisionMask = toUInt(value, 0, Block::MaxValue);
    } else if (key == "block_elevation_mask") {
        this->blockElevationMask = toUInt(value, 0, Block::MaxValue);
    } else if (key == "unused_tile_normal") {
        this->unusedTileNormal = toUInt(value, 0, Tile::MaxValue);
    } else if (key == "unused_tile_covered") {
        this->unusedTileCovered = toUInt(value, 0, Tile::MaxValue);
    } else if (key == "unused_tile_split") {
        this->unusedTileSplit = toUInt(value, 0, Tile::MaxValue);
    } else if (key == "enable_map_allow_flags") {
        this->mapAllowFlagsEnabled = toBool(value);
    } else if (key.startsWith("path/")) {
        this->setFilePath(key.mid(QStringLiteral("path/").length()), value);
    } else if (key.startsWith("ident/")) {
        this->setIdentifier(key.mid(QStringLiteral("ident/").length()), value);
    } else if (key.startsWith("global_constant/")) {
        this->globalConstants.insert(key.mid(QStringLiteral("global_constant/").length()), value);
    } else if (key == "global_constants_filepaths") {
        this->globalConstantsFilepaths = value.split(",", Qt::SkipEmptyParts);
    } else if (key == "tilesets_have_callback") {
        this->tilesetsHaveCallback = toBool(value);
    } else if (key == "tilesets_have_is_compressed") {
        this->tilesetsHaveIsCompressed = toBool(value);
    } else if (key == "set_transparent_pixels_black") { // Old setting replaced by transparency_color
        this->transparencyColor = toBool(value) ? QColor(Qt::black) : QColor();
    } else if (key == "transparency_color") {
        this->transparencyColor = toColor(value);
    } else if (key == "preserve_matching_only_data") {
        this->preserveMatchingOnlyData = toBool(value);
    } else if (key == "event_icon_path_object") {
        this->eventIconPaths[Event::Group::Object] = value;
    } else if (key == "event_icon_path_warp") {
        this->eventIconPaths[Event::Group::Warp] = value;
    } else if (key == "event_icon_path_coord") {
        this->eventIconPaths[Event::Group::Coord] = value;
    } else if (key == "event_icon_path_bg") {
        this->eventIconPaths[Event::Group::Bg] = value;
    } else if (key == "event_icon_path_heal") {
        this->eventIconPaths[Event::Group::Heal] = value;
    } else if (key.startsWith("pokemon_icon_path/")) {
        this->pokemonIconPaths.insert(key.mid(QStringLiteral("pokemon_icon_path/").length()), value);
    } else if (key == "events_tab_icon_path") {
        this->eventsTabIconPath = value;
    } else if (key == "collision_sheet_path") {
        this->collisionSheetPath = value;
    } else if (key == "collision_sheet_width") {
        this->collisionSheetSize.setWidth(toInt(value, 1, Block::MaxValue));
    } else if (key == "collision_sheet_height") {
        this->collisionSheetSize.setHeight(toInt(value, 1, Block::MaxValue));
    } else if (key == "player_view_north") {
        this->playerViewDistance.setTop(toInt(value, 0, INT_MAX, GBA_V_DIST_TO_CENTER));
    } else if (key == "player_view_south") {
        this->playerViewDistance.setBottom(toInt(value, 0, INT_MAX, GBA_V_DIST_TO_CENTER));
    } else if (key == "player_view_west") {
        this->playerViewDistance.setLeft(toInt(value, 0, INT_MAX, GBA_H_DIST_TO_CENTER));
    } else if (key == "player_view_east") {
        this->playerViewDistance.setRight(toInt(value, 0, INT_MAX, GBA_H_DIST_TO_CENTER));
    } else if (key == "warp_behaviors") {
        this->warpBehaviors.clear();
        value.remove(" ");
        const QStringList behaviorList = value.split(",", Qt::SkipEmptyParts);
        for (auto s : behaviorList)
            this->warpBehaviors.insert(toUInt(s));
    } else if (key == "max_events_per_group") {
        this->maxEventsPerGroup = toInt(value, 1, INT_MAX, 255);
    } else if (key == "metatile_selector_width") {
        this->metatileSelectorWidth = toInt(value, 1, INT_MAX, 8);
    } else {
        return false;
    }
    return true;
}

// Read input from the config to get the script paths and whether each is enabled or disbled.
// The format is a comma-separated list of paths. Each path can be followed (before the comma)
// by a :0 or :1 to indicate whether it should be disabled or enabled, respectively. If neither
// follow, it's assumed the script should be enabled.
QList<ScriptSettings> parseCustomScripts(const QString &input) {
    QMap<QString,bool> customScripts;
    const QList<QString> paths = input.split(",", Qt::SkipEmptyParts);
    for (QString path : paths) {
        // Read and remove suffix
        bool enabled = !path.endsWith(":0");
        if (!enabled || path.endsWith(":1"))
            path.chop(2);

        if (!path.isEmpty()) {
            // If a path is repeated only its last instance will be considered.
            customScripts.insert(path, enabled);
        }
    }
    QList<ScriptSettings> settingsList;
    for (auto it = customScripts.begin(); it != customScripts.end(); it++) {
        settingsList.append({
            .path = it.key(),
            .enabled = it.value(),
            .userOnly = true,
        });
    }
    return settingsList;
}

bool UserConfig::parseLegacyKeyValue(const QString &key, const QString &value) {
    if (key == "recent_map_or_layout") {
        this->recentMapOrLayout = value;
    } else if (key == "use_encounter_json") {
        this->useEncounterJson = toBool(value);
    } else if (key == "custom_scripts") {
        this->customScripts = parseCustomScripts(value);
    } else {
        return false;
    }
    return true;
}

bool ShortcutsConfig::parseLegacyKeyValue(const QString &key, const QString &value) {
    QStringList keySequences = value.split(' ');
    for (auto keySequence : keySequences)
        user_shortcuts.insert(key, keySequence);
    return true;
}
