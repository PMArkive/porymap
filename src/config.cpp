#include "config.h"
#include "shortcut.h"
#include "map.h"
#include "validator.h"
#include "utility.h"
#include "metatile.h"
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QList>
#include <QComboBox>
#include <QLabel>
#include <QTextStream>
#include <QRegularExpression>
#include <QAction>
#include <QAbstractButton>
#include <QMainWindow>
#include <QSplitter>

const QVersionNumber porymapVersion = QVersionNumber::fromString(PORYMAP_VERSION);

const QMap<ProjectIdentifier, QPair<QString, QString>> ProjectConfig::defaultIdentifiers = {
    // Symbols
    {ProjectIdentifier::symbol_facing_directions,      {"symbol_facing_directions",      "gInitialMovementTypeFacingDirections"}},
    {ProjectIdentifier::symbol_obj_event_gfx_pointers, {"symbol_obj_event_gfx_pointers", "gObjectEventGraphicsInfoPointers"}},
    {ProjectIdentifier::symbol_pokemon_icon_table,     {"symbol_pokemon_icon_table",     "gMonIconTable"}},
    {ProjectIdentifier::symbol_attribute_table,        {"symbol_attribute_table",        "sMetatileAttrMasks"}},
    {ProjectIdentifier::symbol_tilesets_prefix,        {"symbol_tilesets_prefix",        "gTileset_"}},
    {ProjectIdentifier::symbol_dynamic_map_name,       {"symbol_dynamic_map_name",       "Dynamic"}},
    // Defines
    {ProjectIdentifier::define_obj_event_count,        {"define_obj_event_count",        "OBJECT_EVENT_TEMPLATES_COUNT"}},
    {ProjectIdentifier::define_min_level,              {"define_min_level",              "MIN_LEVEL"}},
    {ProjectIdentifier::define_max_level,              {"define_max_level",              "MAX_LEVEL"}},
    {ProjectIdentifier::define_max_encounter_rate,     {"define_max_encounter_rate",     "MAX_ENCOUNTER_RATE"}},
    {ProjectIdentifier::define_tiles_primary,          {"define_tiles_primary",          "NUM_TILES_IN_PRIMARY"}},
    {ProjectIdentifier::define_tiles_total,            {"define_tiles_total",            "NUM_TILES_TOTAL"}},
    {ProjectIdentifier::define_metatiles_primary,      {"define_metatiles_primary",      "NUM_METATILES_IN_PRIMARY"}},
    {ProjectIdentifier::define_pals_primary,           {"define_pals_primary",           "NUM_PALS_IN_PRIMARY"}},
    {ProjectIdentifier::define_pals_total,             {"define_pals_total",             "NUM_PALS_TOTAL"}},
    {ProjectIdentifier::define_tiles_per_metatile,     {"define_tiles_per_metatile",     "NUM_TILES_PER_METATILE"}},
    {ProjectIdentifier::define_map_size,               {"define_map_size",               "MAX_MAP_DATA_SIZE"}},
    {ProjectIdentifier::define_map_offset_width,       {"define_map_offset_width",       "MAP_OFFSET_W"}},
    {ProjectIdentifier::define_map_offset_height,      {"define_map_offset_height",      "MAP_OFFSET_H"}},
    {ProjectIdentifier::define_mask_metatile,          {"define_mask_metatile",          "MAPGRID_METATILE_ID_MASK"}},
    {ProjectIdentifier::define_mask_collision,         {"define_mask_collision",         "MAPGRID_COLLISION_MASK"}},
    {ProjectIdentifier::define_mask_elevation,         {"define_mask_elevation",         "MAPGRID_ELEVATION_MASK"}},
    {ProjectIdentifier::define_mask_behavior,          {"define_mask_behavior",          "METATILE_ATTR_BEHAVIOR_MASK"}},
    {ProjectIdentifier::define_mask_layer,             {"define_mask_layer",             "METATILE_ATTR_LAYER_MASK"}},
    {ProjectIdentifier::define_attribute_behavior,     {"define_attribute_behavior",     "METATILE_ATTRIBUTE_BEHAVIOR"}},
    {ProjectIdentifier::define_attribute_layer,        {"define_attribute_layer",        "METATILE_ATTRIBUTE_LAYER_TYPE"}},
    {ProjectIdentifier::define_attribute_terrain,      {"define_attribute_terrain",      "METATILE_ATTRIBUTE_TERRAIN"}},
    {ProjectIdentifier::define_attribute_encounter,    {"define_attribute_encounter",    "METATILE_ATTRIBUTE_ENCOUNTER_TYPE"}},
    {ProjectIdentifier::define_metatile_label_prefix,  {"define_metatile_label_prefix",  "METATILE_"}},
    {ProjectIdentifier::define_heal_locations_prefix,  {"define_heal_locations_prefix",  "HEAL_LOCATION_"}},
    {ProjectIdentifier::define_layout_prefix,          {"define_layout_prefix",          "LAYOUT_"}},
    {ProjectIdentifier::define_map_prefix,             {"define_map_prefix",             "MAP_"}},
    {ProjectIdentifier::define_map_dynamic,            {"define_map_dynamic",            "MAP_DYNAMIC"}},
    {ProjectIdentifier::define_map_empty,              {"define_map_empty",              "MAP_UNDEFINED"}},
    {ProjectIdentifier::define_map_section_prefix,     {"define_map_section_prefix",     "MAPSEC_"}},
    {ProjectIdentifier::define_map_section_empty,      {"define_map_section_empty",      "NONE"}},
    {ProjectIdentifier::define_species_prefix,         {"define_species_prefix",         "SPECIES_"}},
    {ProjectIdentifier::define_species_empty,          {"define_species_empty",          "NONE"}},
    // Regex
    {ProjectIdentifier::regex_behaviors,               {"regex_behaviors",               "\\bMB_"}},
    {ProjectIdentifier::regex_obj_event_gfx,           {"regex_obj_event_gfx",           "\\bOBJ_EVENT_GFX_"}},
    {ProjectIdentifier::regex_items,                   {"regex_items",                   "\\bITEM_(?!(B_)?USE_)"}}, // Exclude ITEM_USE_ and ITEM_B_USE_ constants
    {ProjectIdentifier::regex_flags,                   {"regex_flags",                   "\\bFLAG_"}},
    {ProjectIdentifier::regex_vars,                    {"regex_vars",                    "\\bVAR_"}},
    {ProjectIdentifier::regex_movement_types,          {"regex_movement_types",          "\\bMOVEMENT_TYPE_"}},
    {ProjectIdentifier::regex_map_types,               {"regex_map_types",               "\\bMAP_TYPE_"}},
    {ProjectIdentifier::regex_battle_scenes,           {"regex_battle_scenes",           "\\bMAP_BATTLE_SCENE_"}},
    {ProjectIdentifier::regex_weather,                 {"regex_weather",                 "\\bWEATHER_"}},
    {ProjectIdentifier::regex_coord_event_weather,     {"regex_coord_event_weather",     "\\bCOORD_EVENT_WEATHER_"}},
    {ProjectIdentifier::regex_secret_bases,            {"regex_secret_bases",            "\\bSECRET_BASE_[\\w]+_[\\d]+"}},
    {ProjectIdentifier::regex_sign_facing_directions,  {"regex_sign_facing_directions",  "\\bBG_EVENT_PLAYER_FACING_"}},
    {ProjectIdentifier::regex_trainer_types,           {"regex_trainer_types",           "\\bTRAINER_TYPE_"}},
    {ProjectIdentifier::regex_music,                   {"regex_music",                   "\\b(SE|MUS)_"}},
    {ProjectIdentifier::regex_encounter_types,         {"regex_encounter_types",         "\\bTILE_ENCOUNTER_"}},
    {ProjectIdentifier::regex_terrain_types,           {"regex_terrain_types",           "\\bTILE_TERRAIN_"}},
    // Other
    {ProjectIdentifier::pals_output_extension,         {"pals_output_extension",         ".gbapal"}},
    {ProjectIdentifier::tiles_output_extension,        {"tiles_output_extension",        ".4bpp.lz"}},
};

const QMap<ProjectFilePath, QPair<QString, QString>> ProjectConfig::defaultPaths = {
    {ProjectFilePath::data_map_folders,                 { "data_map_folders",                "data/maps/"}},
    {ProjectFilePath::data_scripts_folders,             { "data_scripts_folders",            "data/scripts/"}},
    {ProjectFilePath::data_layouts_folders,             { "data_layouts_folders",            "data/layouts/"}},
    {ProjectFilePath::data_primary_tilesets_folders,    { "data_primary_tilesets_folders",   "data/tilesets/primary/"}},
    {ProjectFilePath::data_secondary_tilesets_folders,  { "data_secondary_tilesets_folders", "data/tilesets/secondary/"}},
    {ProjectFilePath::data_event_scripts,               { "data_event_scripts",              "data/event_scripts.s"}},
    {ProjectFilePath::json_map_groups,                  { "json_map_groups",                 "data/maps/map_groups.json"}},
    {ProjectFilePath::json_layouts,                     { "json_layouts",                    "data/layouts/layouts.json"}},
    {ProjectFilePath::json_wild_encounters,             { "json_wild_encounters",            "src/data/wild_encounters.json"}},
    {ProjectFilePath::json_heal_locations,              { "json_heal_locations",             "src/data/heal_locations.json"}},
    {ProjectFilePath::json_region_map_entries,          { "json_region_map_entries",         "src/data/region_map/region_map_sections.json"}},
    {ProjectFilePath::json_region_porymap_cfg,          { "json_region_porymap_cfg",         "src/data/region_map/porymap_config.json"}},
    {ProjectFilePath::tilesets_headers,                 { "tilesets_headers",                "src/data/tilesets/headers.h"}},
    {ProjectFilePath::tilesets_graphics,                { "tilesets_graphics",               "src/data/tilesets/graphics.h"}},
    {ProjectFilePath::tilesets_metatiles,               { "tilesets_metatiles",              "src/data/tilesets/metatiles.h"}},
    {ProjectFilePath::tilesets_headers_asm,             { "tilesets_headers_asm",            "data/tilesets/headers.inc"}},
    {ProjectFilePath::tilesets_graphics_asm,            { "tilesets_graphics_asm",           "data/tilesets/graphics.inc"}},
    {ProjectFilePath::tilesets_metatiles_asm,           { "tilesets_metatiles_asm",          "data/tilesets/metatiles.inc"}},
    {ProjectFilePath::data_obj_event_gfx_pointers,      { "data_obj_event_gfx_pointers",     "src/data/object_events/object_event_graphics_info_pointers.h"}},
    {ProjectFilePath::data_obj_event_gfx_info,          { "data_obj_event_gfx_info",         "src/data/object_events/object_event_graphics_info.h"}},
    {ProjectFilePath::data_obj_event_pic_tables,        { "data_obj_event_pic_tables",       "src/data/object_events/object_event_pic_tables.h"}},
    {ProjectFilePath::data_obj_event_gfx,               { "data_obj_event_gfx",              "src/data/object_events/object_event_graphics.h"}},
    {ProjectFilePath::data_pokemon_gfx,                 { "data_pokemon_gfx",                "src/data/graphics/pokemon.h"}},
    {ProjectFilePath::constants_global,                 { "constants_global",                "include/constants/global.h"}},
    {ProjectFilePath::constants_items,                  { "constants_items",                 "include/constants/items.h"}},
    {ProjectFilePath::constants_flags,                  { "constants_flags",                 "include/constants/flags.h"}},
    {ProjectFilePath::constants_vars,                   { "constants_vars",                  "include/constants/vars.h"}},
    {ProjectFilePath::constants_weather,                { "constants_weather",               "include/constants/weather.h"}},
    {ProjectFilePath::constants_songs,                  { "constants_songs",                 "include/constants/songs.h"}},
    {ProjectFilePath::constants_pokemon,                { "constants_pokemon",               "include/constants/pokemon.h"}},
    {ProjectFilePath::constants_map_types,              { "constants_map_types",             "include/constants/map_types.h"}},
    {ProjectFilePath::constants_trainer_types,          { "constants_trainer_types",         "include/constants/trainer_types.h"}},
    {ProjectFilePath::constants_secret_bases,           { "constants_secret_bases",          "include/constants/secret_bases.h"}},
    {ProjectFilePath::constants_obj_event_movement,     { "constants_obj_event_movement",    "include/constants/event_object_movement.h"}},
    {ProjectFilePath::constants_obj_events,             { "constants_obj_events",            "include/constants/event_objects.h"}},
    {ProjectFilePath::constants_event_bg,               { "constants_event_bg",              "include/constants/event_bg.h"}},
    {ProjectFilePath::constants_metatile_labels,        { "constants_metatile_labels",       "include/constants/metatile_labels.h"}},
    {ProjectFilePath::constants_metatile_behaviors,     { "constants_metatile_behaviors",    "include/constants/metatile_behaviors.h"}},
    {ProjectFilePath::constants_species,                { "constants_species",               "include/constants/species.h"}},
    {ProjectFilePath::constants_fieldmap,               { "constants_fieldmap",              "include/fieldmap.h"}},
    {ProjectFilePath::global_fieldmap,                  { "global_fieldmap",                 "include/global.fieldmap.h"}},
    {ProjectFilePath::fieldmap,                         { "fieldmap",                        "src/fieldmap.c"}},
    {ProjectFilePath::pokemon_icon_table,               { "pokemon_icon_table",              "src/pokemon_icon.c"}},
    {ProjectFilePath::initial_facing_table,             { "initial_facing_table",            "src/event_object_movement.c"}},
    {ProjectFilePath::wild_encounter,                   { "wild_encounter",                  "src/wild_encounter.c"}},
    {ProjectFilePath::pokemon_gfx,                      { "pokemon_gfx",                     "graphics/pokemon/"}},
};

ProjectIdentifier ProjectConfig::reverseDefaultIdentifier(const QString& str) {
    for (auto i = defaultIdentifiers.cbegin(), end = defaultIdentifiers.cend(); i != end; i++) {
        if (i.value().first == str) return i.key();
    }
    return static_cast<ProjectIdentifier>(-1);
}

ProjectFilePath ProjectConfig::reverseDefaultPaths(const QString& str) {
    for (auto it = defaultPaths.constKeyValueBegin(); it != defaultPaths.constKeyValueEnd(); ++it) {
        if ((*it).second.first == str) return (*it).first;
    }
    return static_cast<ProjectFilePath>(-1);
}

void KeyValueConfigBase::setRoot(const QString& root) {
    m_root = root;
    QDir dir(m_root);
    if (!m_root.isEmpty() && !dir.exists()) {
        dir.mkpath(m_root);
    }
    // Caching the filepath constructed from m_root + m_filename
    m_filepath = dir.absoluteFilePath(m_filename);
}

bool KeyValueConfigBase::load() {
    QFile file(filepath());
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            logError(QString("Failed to read config file '%1': %2").arg(filepath()).arg(file.errorString()));
            return false;
        }
        if (file.size() == 0) {
            logWarn(QString("Config file '%1' was empty.").arg(filepath()));
            // An empty file isn't a valid JSON file, but other than the warning
            // we'll treat it the same as if it were a JSON file with an empty object.
            initializeFromEmpty();
            return true;
        }
        QJsonParseError parseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            logError(QString("Failed to read config file '%1': %2").arg(filepath()).arg(parseError.errorString()));
            return false;
        }
        if (!jsonDoc.isObject()) {
            logError(QString("Failed to read config file '%1': Expected top level JSON object.").arg(filepath()));
            return false;
        }
        loadFromJson(jsonDoc.object());
        logInfo(QString("Loaded config file '%1'").arg(filename()));
    } else if (!loadLegacy()) {
        // No config file is present (either in the new or old format)
        initializeFromEmpty();
    }
    return true;
}

void KeyValueConfigBase::loadFromJson(const QJsonObject& obj) {
    for (auto it = obj.begin(); it != obj.end(); it++) {
        if (!parseJsonKeyValue(it.key(), it.value())) {
            logWarn(QString("Discarding unrecognized config key '%1'").arg(it.key()));
        }
    }
}

bool KeyValueConfigBase::parseJsonKeyValue(const QString& key, const QJsonValue& value) {
    const auto fields = registeredFields();
    auto it = fields.find(key);
    if (it == fields.end()) return false;

    // Recognized 'key' as a registered field. Let the FieldManager try to assign the value.
    const QStringList errors = it.value().set(this, value);
    if (errors.length() == 1)     logWarn(QString("Failed to read config key '%1': %2").arg(key).arg(errors.at(0)));
    else if (errors.length() > 1) logWarn(QString("Failed to read config key '%1':\n%2").arg(key).arg(errors.join("\n")));
    return true;
}

QJsonObject KeyValueConfigBase::toJson() const {
    QJsonObject obj;
    const auto fields = registeredFields();
    for (auto it = fields.begin(); it != fields.end(); it++) {
        obj[it.key()] = it.value().get(this);
    }
    return obj;
}

bool KeyValueConfigBase::save() {
    QFile file(filepath());
    if (!file.open(QIODevice::WriteOnly)) {
        logError(QString("Could not open config file '%1' for writing: ").arg(filepath()) + file.errorString());
        return false;
    }
   
    QJsonObject savedObject;
    if (m_saveAllFields) {
        savedObject = toJson();
    } else {
        // We limit the output to fields that have changed from the default value.
        // This has a few notable benefits:
        // - It allows changes to the default values to be downstreamed from Porymap.
        // - It reduces diff noise for configs as Porymap's settings change over time.
        // - It discourages manual editing of the file; all settings should be edited in the GUI.
        // If the child class does not reimplement getDefaultJson it returns an empty QJsonObject,
        // and so the default behavior is to output all fields.
        const QJsonObject curObject = toJson();
        const QJsonObject defaultObject = getDefaultJson();
        for (auto it = curObject.begin(); it != curObject.end(); it++) {
            if (it.value() != defaultObject.value(it.key())) {
                savedObject[it.key()] = it.value();
            }
        }
    }

    QJsonDocument doc(savedObject);
    file.write(doc.toJson());
    return true;
}

PorymapConfig porymapConfig;

bool PorymapConfig::save()  {
    // Clean out old rate limit times, leaving only times still in the future.
    for (auto it = this->rateLimitTimes.begin(); it != this->rateLimitTimes.end();) {
        const QDateTime time = it.value();
        if (!time.isNull() && time > QDateTime::currentDateTime()) {
            it = this->rateLimitTimes.erase(it);
        } else it++;
    }

    return KeyValueConfigBase::save();
}

void PorymapConfig::loadFromJson(const QJsonObject& obj) {
    KeyValueConfigBase::loadFromJson(obj);

    // Reset geometry between major/minor versions.
    // We could try to keep separate versions for each geometry,
    // but that requires a lot of careful maintenance.
    // This ensures that as widgets change they won't
    // receive data for old layouts/states, and that as widgets
    // get renamed their old keys wont accumulate in the config.
    constexpr int CurrentGeometryVersion = 1;
    if (this->geometryVersion != CurrentGeometryVersion) {
        this->geometryVersion = CurrentGeometryVersion;
        this->savedGeometryMap.clear();
    }
}

QJsonObject PorymapConfig::getDefaultJson() const {
    PorymapConfig defaultConfig;
    return defaultConfig.toJson();
}

void PorymapConfig::addRecentProject(const QString& project) {
    this->recentProjects.removeOne(project);
    this->recentProjects.prepend(project);
}

void PorymapConfig::setRecentProjects(const QStringList& projects) {
    this->recentProjects = projects;
}

QString PorymapConfig::getRecentProject() const {
    return this->recentProjects.value(0);
}

const QStringList& PorymapConfig::getRecentProjects() const {
    return this->recentProjects;
}

void PorymapConfig::saveGeometry(const QWidget* widget, const QString& keyPrefix, bool recursive) {
    if (!widget || widget->objectName().isEmpty()) return;

    const QString key = keyPrefix + widget->objectName();
    this->savedGeometryMap.insert(key, widget->saveGeometry());

    // In addition to geometry, some widgets have other states that can be saved/restored.
    const QString stateKey = key + QStringLiteral("/State");
    auto mainWindow = qobject_cast<const QMainWindow*>(widget);
    if (mainWindow) this->savedGeometryMap.insert(stateKey, mainWindow->saveState());
    else {
        auto splitter = qobject_cast<const QSplitter*>(widget);
        if (splitter) this->savedGeometryMap.insert(stateKey, splitter->saveState());
    }
    if (recursive) {
        for (const auto splitter : widget->findChildren<QSplitter*>()) {
            saveGeometry(splitter, key + "_", false);
        }
    }
}

bool PorymapConfig::restoreGeometry(QWidget* widget, const QString& keyPrefix, bool recursive) const {
    if (!widget || widget->objectName().isEmpty()) return false;

    const QString key = keyPrefix + widget->objectName();
    auto it = this->savedGeometryMap.constFind(key);
    if (it == this->savedGeometryMap.constEnd()) return false;
    widget->restoreGeometry(it.value());

    // In addition to geometry, some widgets have other states that can be saved/restored.
    it = this->savedGeometryMap.constFind(key + QStringLiteral("/State"));
    if (it != this->savedGeometryMap.constEnd()) {
        auto mainWindow = qobject_cast<QMainWindow*>(widget);
        if (mainWindow) mainWindow->restoreState(it.value());
        else {
            auto splitter = qobject_cast<QSplitter*>(widget);
            if (splitter) splitter->restoreState(it.value());
        }
    }
    if (recursive) {
        for (const auto splitter : widget->findChildren<QSplitter*>()) {
            restoreGeometry(splitter, key + "_", false);
        }
    }
    return true;
}

// TODO: This should eventually be contained by each individual Project instance.
ProjectConfig projectConfig;

void ProjectConfig::setVersionSpecificDefaults(BaseGame::Version version) {
    this->baseGameVersion = version;
    if (this->baseGameVersion == BaseGame::Version::none) return;

    this->metatileAttributesSize = Metatile::getDefaultAttributesSize(this->baseGameVersion);
    this->metatileBehaviorMask = Metatile::getDefaultAttributesMask(this->baseGameVersion, Metatile::Attr::Behavior);
    this->metatileTerrainTypeMask = Metatile::getDefaultAttributesMask(this->baseGameVersion, Metatile::Attr::TerrainType);
    this->metatileEncounterTypeMask = Metatile::getDefaultAttributesMask(this->baseGameVersion, Metatile::Attr::EncounterType);
    this->metatileLayerTypeMask = Metatile::getDefaultAttributesMask(this->baseGameVersion, Metatile::Attr::LayerType);

    if (this->baseGameVersion == BaseGame::Version::pokefirered) {
        this->useCustomBorderSize = true;
        this->eventWeatherTriggerEnabled = false;
        this->eventSecretBaseEnabled = false;
        this->hiddenItemQuantityEnabled = true;
        this->hiddenItemRequiresItemfinderEnabled = true;
        this->healLocationRespawnDataEnabled = true;
        this->eventCloneObjectEnabled = true;
        this->floorNumberEnabled = true;
        this->createMapTextFileEnabled = true;
        this->newMapBorderMetatileIds = {0x14, 0x15, 0x1C, 0x1D};
        this->defaultSecondaryTileset = QStringLiteral("gTileset_PalletTown");
        this->warpBehaviors = {
            0x60, // MB_CAVE_DOOR
            0x61, // MB_LADDER
            0x62, // MB_EAST_ARROW_WARP
            0x63, // MB_WEST_ARROW_WARP
            0x64, // MB_NORTH_ARROW_WARP
            0x65, // MB_SOUTH_ARROW_WARP
            0x66, // MB_FALL_WARP
            0x67, // MB_REGULAR_WARP
            0x68, // MB_LAVARIDGE_1F_WARP
            0x69, // MB_WARP_DOOR
            0x6A, // MB_UP_ESCALATOR
            0x6B, // MB_DOWN_ESCALATOR
            0x6C, // MB_UP_RIGHT_STAIR_WARP
            0x6D, // MB_UP_LEFT_STAIR_WARP
            0x6E, // MB_DOWN_RIGHT_STAIR_WARP
            0x6F, // MB_DOWN_LEFT_STAIR_WARP
            0x71, // MB_UNION_ROOM_WARP
        };
    } else { // pokeemerald / pokeruby
        this->useCustomBorderSize = false;
        this->eventWeatherTriggerEnabled = true;
        this->eventSecretBaseEnabled = true;
        this->hiddenItemQuantityEnabled = false;
        this->hiddenItemRequiresItemfinderEnabled = false;
        this->healLocationRespawnDataEnabled = false;
        this->eventCloneObjectEnabled = false;
        this->floorNumberEnabled = false;
        this->createMapTextFileEnabled = false;
        this->newMapBorderMetatileIds = {0x1D4, 0x1D5, 0x1DC, 0x1DD};
        this->defaultSecondaryTileset = QStringLiteral("gTileset_Petalburg");
        this->warpBehaviors = {
            0x0E, // MB_MOSSDEEP_GYM_WARP
            0x0F, // MB_MT_PYRE_HOLE
            0x1B, // MB_STAIRS_OUTSIDE_ABANDONED_SHIP
            0x1C, // MB_SHOAL_CAVE_ENTRANCE
            0x29, // MB_LAVARIDGE_GYM_B1F_WARP
            0x60, // MB_NON_ANIMATED_DOOR
            0x61, // MB_LADDER
            0x62, // MB_EAST_ARROW_WARP
            0x63, // MB_WEST_ARROW_WARP
            0x64, // MB_NORTH_ARROW_WARP
            0x65, // MB_SOUTH_ARROW_WARP
            0x67, // MB_AQUA_HIDEOUT_WARP
            0x68, // MB_LAVARIDGE_GYM_1F_WARP
            0x69, // MB_ANIMATED_DOOR
            0x6A, // MB_UP_ESCALATOR
            0x6B, // MB_DOWN_ESCALATOR
            0x6C, // MB_WATER_DOOR
            0x6D, // MB_WATER_SOUTH_ARROW_WARP
            0x6E, // MB_DEEP_SOUTH_WARP
            0x70, // MB_UNION_ROOM_WARP
            0x8D, // MB_PETALBURG_GYM_DOOR
            0x91, // MB_SECRET_BASE_SPOT_RED_CAVE_OPEN
            0x93, // MB_SECRET_BASE_SPOT_BROWN_CAVE_OPEN
            0x95, // MB_SECRET_BASE_SPOT_YELLOW_CAVE_OPEN
            0x97, // MB_SECRET_BASE_SPOT_TREE_LEFT_OPEN
            0x99, // MB_SECRET_BASE_SPOT_SHRUB_OPEN
            0x9B, // MB_SECRET_BASE_SPOT_BLUE_CAVE_OPEN
            0x9D, // MB_SECRET_BASE_SPOT_TREE_RIGHT_OPEN
        };
        if (this->baseGameVersion == BaseGame::Version::pokeruby) {
            this->mapAllowFlagsEnabled = false;
        }
    }
}

bool ProjectConfig::save()  {
    // Clean out empty paths
    Util::removeEmptyStrings(&this->globalConstants);
    Util::removeEmptyStrings(&this->eventIconPaths);
    Util::removeEmptyStrings(&this->pokemonIconPaths);

    return KeyValueConfigBase::save();
}

void ProjectConfig::loadFromJson(const QJsonObject& obj_) {
    QJsonObject obj(obj_);

    // Parse the base game version before anything else so we can initialize the defaults.
    auto versionKey = QStringLiteral("base_game_version");
    const QJsonValue version = obj.take(versionKey);
    if (!version.isUndefined() && parseJsonKeyValue(versionKey, version)) {
        setVersionSpecificDefaults(this->baseGameVersion);
    }

    KeyValueConfigBase::loadFromJson(obj);

    // Enforce this setting for projectConfig's custom scripts
    for (auto& settings : this->customScripts) settings.userOnly = false;
}

QJsonObject ProjectConfig::getDefaultJson() const {
    ProjectConfig defaultConfig(this->baseGameVersion);
    // The defaults are version-specific, make sure we always output non-empty versions.
    defaultConfig.baseGameVersion = BaseGame::Version::none;
    return defaultConfig.toJson();
}

void ProjectConfig::initializeFromEmpty() {
    const QString dirName = QDir(projectDir()).dirName();
    BaseGame::Version version = BaseGame::stringToVersion(dirName);
    if (version != BaseGame::Version::none) {
        this->baseGameVersion = version;
        logInfo(QString("Auto-detected base_game_version as '%1'").arg(BaseGame::versionToString(version)));
    } else {
        QDialog dialog(nullptr, Qt::WindowTitleHint);
        dialog.setWindowTitle("Project Configuration");
        dialog.setWindowModality(Qt::NonModal);

        QFormLayout form(&dialog);

        QComboBox *baseGameVersionComboBox = new QComboBox();
        // TODO: Populate dynamically, same as project settings editor
        baseGameVersionComboBox->addItem("pokeruby", BaseGame::Version::pokeruby);
        baseGameVersionComboBox->addItem("pokefirered", BaseGame::Version::pokefirered);
        baseGameVersionComboBox->addItem("pokeemerald", BaseGame::Version::pokeemerald);
        form.addRow(new QLabel("Game Version"), baseGameVersionComboBox);

        // TODO: Add an 'Advanced' button to open the project settings window (with some settings disabled)

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
        QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        form.addRow(&buttonBox);

        if (dialog.exec() == QDialog::Accepted) {
            this->baseGameVersion = static_cast<BaseGame::Version>(baseGameVersionComboBox->currentData().toInt());
        } else {
            logWarn(QString("No base_game_version selected, using default '%1'").arg(BaseGame::versionToString(this->baseGameVersion)));
        }
    }
}

void ProjectConfig::setFilePath(ProjectFilePath pathId, const QString& path) {
    if (!defaultPaths.contains(pathId)) return;
    if (path.isEmpty()) {
        this->filePaths.remove(pathId);
    } else {
        this->filePaths[pathId] = path;
    }
}

void ProjectConfig::setFilePath(const QString& pathId, const QString& path) {
    this->setFilePath(reverseDefaultPaths(pathId), path);
}

QString ProjectConfig::getCustomFilePath(ProjectFilePath pathId) {
    return QDir::cleanPath(this->filePaths.value(pathId));
}

QString ProjectConfig::getCustomFilePath(const QString& pathId) {
    return this->getCustomFilePath(reverseDefaultPaths(pathId));
}

QString ProjectConfig::getFilePath(ProjectFilePath pathId) {
    QString customPath = this->getCustomFilePath(pathId);
    if (!customPath.isEmpty()) {
        // A custom filepath has been specified. If the file/folder exists, use that.
        const QString baseDir = this->projectDir() + "/";
        if (customPath.startsWith(baseDir)) {
            customPath.remove(0, baseDir.length());
        }
        if (QFileInfo::exists(QDir::cleanPath(baseDir + customPath))) {
            return customPath;
        } else {
            logError(QString("Custom project filepath '%1' not found. Using default.").arg(customPath));
        }
    }
    return defaultPaths.contains(pathId) ? defaultPaths[pathId].second : QString();
}

void ProjectConfig::setIdentifier(ProjectIdentifier id, const QString& text) {
    if (!defaultIdentifiers.contains(id))
        return;

    if (text.isEmpty()) {
        this->identifiers.remove(id);
    } else {
        const QString idName = defaultIdentifiers.value(id).first;
        if (idName.startsWith("define_") || idName.startsWith("symbol_")) {
            // Validate the input for the identifier, depending on the type.
            IdentifierValidator validator;
            if (!validator.isValid(text)) {
                logError(QString("The name '%1' for project identifier '%2' is invalid. It must only contain word characters, and cannot start with a digit.").arg(text).arg(idName));
                return;
            }
        }
        this->identifiers[id] = text;
    }
}

void ProjectConfig::setIdentifier(const QString& id, const QString& text) {
    this->setIdentifier(reverseDefaultIdentifier(id), text);
}

QString ProjectConfig::getCustomIdentifier(ProjectIdentifier id) {
    return this->identifiers.value(id);
}

QString ProjectConfig::getCustomIdentifier(const QString& id) {
    return this->getCustomIdentifier(reverseDefaultIdentifier(id));
}

QString ProjectConfig::getIdentifier(ProjectIdentifier id) {
    const QString customText = this->getCustomIdentifier(id);
    if (!customText.isEmpty())
        return customText;
    return defaultIdentifiers.contains(id) ? defaultIdentifiers[id].second : QString();
}

// TODO: This should eventually be contained by each individual Project instance.
UserConfig userConfig;

void UserConfig::loadFromJson(const QJsonObject& obj) {
    KeyValueConfigBase::loadFromJson(obj);

    // Enforce this setting for userConfig's custom scripts
    for (auto& settings : this->customScripts) settings.userOnly = true;
}

QJsonObject UserConfig::getDefaultJson() const {
    UserConfig defaultConfig;
    return defaultConfig.toJson();
}

ShortcutsConfig shortcutsConfig;

void ShortcutsConfig::loadFromJson(const QJsonObject& obj) {
    this->user_shortcuts = Converter<QMultiMap<QString, QKeySequence>>::fromJson(obj);
}

QJsonObject ShortcutsConfig::toJson() const {
    return Converter<QMultiMap<QString, QKeySequence>>::toJson(this->user_shortcuts);
}

QJsonObject ShortcutsConfig::getDefaultJson() const {
    return Converter<QMultiMap<QString, QKeySequence>>::toJson(this->default_shortcuts);
}

void ShortcutsConfig::setDefaultShortcuts(const QObjectList& objects) {
    storeShortcutsFromList(StoreType::Default, objects);
}

QList<QKeySequence> ShortcutsConfig::defaultShortcuts(const QObject *object) const {
    return default_shortcuts.values(cfgKey(object));
}

void ShortcutsConfig::setUserShortcuts(const QObjectList& objects) {
    storeShortcutsFromList(StoreType::User, objects);
}

void ShortcutsConfig::setUserShortcuts(const QMultiMap<const QObject *, QKeySequence>& objects_keySequences) {
    for (auto *object : objects_keySequences.uniqueKeys())
        if (!object->objectName().isEmpty() && !object->objectName().startsWith("_q_"))
            storeShortcuts(StoreType::User, cfgKey(object), objects_keySequences.values(object));
}

QList<QKeySequence> ShortcutsConfig::userShortcuts(const QObject *object) const {
    return user_shortcuts.values(cfgKey(object));
}

void ShortcutsConfig::storeShortcutsFromList(StoreType storeType, const QObjectList& objects) {
    for (const auto *object : objects)
        if (!object->objectName().isEmpty() && !object->objectName().startsWith("_q_"))
            storeShortcuts(storeType, cfgKey(object), currentShortcuts(object));
}

void ShortcutsConfig::storeShortcuts(
        StoreType storeType,
        const QString& cfgKey,
        const QList<QKeySequence>& keySequences)
{
    bool storeUser = (storeType == User) || !user_shortcuts.contains(cfgKey);

    if (storeType == Default)
        default_shortcuts.remove(cfgKey);
    if (storeUser)
        user_shortcuts.remove(cfgKey);

    if (keySequences.isEmpty()) {
        if (storeType == Default)
            default_shortcuts.insert(cfgKey, QKeySequence());
        if (storeUser)
            user_shortcuts.insert(cfgKey, QKeySequence());
    } else {
        for (auto keySequence : keySequences) {
            if (storeType == Default)
                default_shortcuts.insert(cfgKey, keySequence);
            if (storeUser)
                user_shortcuts.insert(cfgKey, keySequence);
        }
    }
}

/* Creates a config key from the object's name prepended with the parent 
 * window's object name, and converts camelCase to snake_case. */
QString ShortcutsConfig::cfgKey(const QObject *object) const {
    auto cfg_key = QString();
    auto *parentWidget = static_cast<QWidget *>(object->parent());
    if (parentWidget)
        cfg_key = parentWidget->window()->objectName() + '_';
    cfg_key += object->objectName();

    static const QRegularExpression re("[A-Z]");
    int i = cfg_key.indexOf(re, 1);
    while (i != -1) {
        if (cfg_key.at(i - 1) != '_')
            cfg_key.insert(i++, '_');
        i = cfg_key.indexOf(re, i + 1);
    }
    return cfg_key.toLower();
}

QList<QKeySequence> ShortcutsConfig::currentShortcuts(const QObject *object) const {
    if (object->inherits("QAction")) {
        const auto *action = qobject_cast<const QAction *>(object);
        return action->shortcuts();
    } else if (object->inherits("Shortcut")) {
        const auto *shortcut = qobject_cast<const Shortcut *>(object);
        return shortcut->keys();
    } else if (object->inherits("QShortcut")) {
        const auto *qshortcut = qobject_cast<const QShortcut *>(object);
        return { qshortcut->key() };
    } else if (object->property("shortcut").isValid()) {
        return { object->property("shortcut").value<QKeySequence>() };
    } else {
        return { };
    }
}
