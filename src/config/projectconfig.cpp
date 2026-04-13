#include "projectconfig.h"
#include "regex.h"
#include "utility.h"
#include "validator.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QLabel>

// TODO: This should eventually be contained by each individual Project instance.
ProjectConfig projectConfig;

const QMap<ProjectIdentifier, QString> ProjectConfig::defaultIdentifiers = {
    // Symbols
    {ProjectIdentifier::symbol_facing_directions,      "gInitialMovementTypeFacingDirections"},
    {ProjectIdentifier::symbol_obj_event_gfx_pointers, "gObjectEventGraphicsInfoPointers"},
    {ProjectIdentifier::symbol_pokemon_icon_table,     "gMonIconTable"},
    {ProjectIdentifier::symbol_attribute_table,        "sMetatileAttrMasks"},
    {ProjectIdentifier::symbol_tilesets_prefix,        "gTileset_"},
    {ProjectIdentifier::symbol_dynamic_map_name,       "Dynamic"},
    // Defines
    {ProjectIdentifier::define_obj_event_count,        "OBJECT_EVENT_TEMPLATES_COUNT"},
    {ProjectIdentifier::define_min_level,              "MIN_LEVEL"},
    {ProjectIdentifier::define_max_level,              "MAX_LEVEL"},
    {ProjectIdentifier::define_max_encounter_rate,     "MAX_ENCOUNTER_RATE"},
    {ProjectIdentifier::define_tiles_primary,          "NUM_TILES_IN_PRIMARY"},
    {ProjectIdentifier::define_tiles_total,            "NUM_TILES_TOTAL"},
    {ProjectIdentifier::define_metatiles_primary,      "NUM_METATILES_IN_PRIMARY"},
    {ProjectIdentifier::define_pals_primary,           "NUM_PALS_IN_PRIMARY"},
    {ProjectIdentifier::define_pals_total,             "NUM_PALS_TOTAL"},
    {ProjectIdentifier::define_tiles_per_metatile,     "NUM_TILES_PER_METATILE"},
    {ProjectIdentifier::define_map_size,               "MAX_MAP_DATA_SIZE"},
    {ProjectIdentifier::define_map_offset_width,       "MAP_OFFSET_W"},
    {ProjectIdentifier::define_map_offset_height,      "MAP_OFFSET_H"},
    {ProjectIdentifier::define_mask_metatile,          "MAPGRID_METATILE_ID_MASK"},
    {ProjectIdentifier::define_mask_collision,         "MAPGRID_COLLISION_MASK"},
    {ProjectIdentifier::define_mask_elevation,         "MAPGRID_ELEVATION_MASK"},
    {ProjectIdentifier::define_mask_behavior,          "METATILE_ATTR_BEHAVIOR_MASK"},
    {ProjectIdentifier::define_mask_layer,             "METATILE_ATTR_LAYER_MASK"},
    {ProjectIdentifier::define_attribute_behavior,     "METATILE_ATTRIBUTE_BEHAVIOR"},
    {ProjectIdentifier::define_attribute_layer,        "METATILE_ATTRIBUTE_LAYER_TYPE"},
    {ProjectIdentifier::define_attribute_terrain,      "METATILE_ATTRIBUTE_TERRAIN"},
    {ProjectIdentifier::define_attribute_encounter,    "METATILE_ATTRIBUTE_ENCOUNTER_TYPE"},
    {ProjectIdentifier::define_metatile_label_prefix,  "METATILE_"},
    {ProjectIdentifier::define_heal_locations_prefix,  "HEAL_LOCATION_"},
    {ProjectIdentifier::define_layout_prefix,          "LAYOUT_"},
    {ProjectIdentifier::define_map_prefix,             "MAP_"},
    {ProjectIdentifier::define_map_dynamic,            "MAP_DYNAMIC"},
    {ProjectIdentifier::define_map_empty,              "MAP_UNDEFINED"},
    {ProjectIdentifier::define_map_section_prefix,     "MAPSEC_"},
    {ProjectIdentifier::define_map_section_empty,      "NONE"},
    {ProjectIdentifier::define_species_prefix,         "SPECIES_"},
    {ProjectIdentifier::define_species_empty,          "NONE"},
    // Regex
    {ProjectIdentifier::regex_behaviors,               "\\bMB_"},
    {ProjectIdentifier::regex_obj_event_gfx,           "\\bOBJ_EVENT_GFX_"},
    {ProjectIdentifier::regex_items,                   "\\bITEM_(?!(B_)?USE_)"}, // Exclude ITEM_USE_ and ITEM_B_USE_ constants
    {ProjectIdentifier::regex_flags,                   "\\bFLAG_"},
    {ProjectIdentifier::regex_vars,                    "\\bVAR_"},
    {ProjectIdentifier::regex_movement_types,          "\\bMOVEMENT_TYPE_"},
    {ProjectIdentifier::regex_map_types,               "\\bMAP_TYPE_"},
    {ProjectIdentifier::regex_battle_scenes,           "\\bMAP_BATTLE_SCENE_"},
    {ProjectIdentifier::regex_weather,                 "\\bWEATHER_"},
    {ProjectIdentifier::regex_coord_event_weather,     "\\bCOORD_EVENT_WEATHER_"},
    {ProjectIdentifier::regex_secret_bases,            "\\bSECRET_BASE_[\\w]+_[\\d]+"},
    {ProjectIdentifier::regex_sign_facing_directions,  "\\bBG_EVENT_PLAYER_FACING_"},
    {ProjectIdentifier::regex_trainer_types,           "\\bTRAINER_TYPE_"},
    {ProjectIdentifier::regex_music,                   "\\b(SE|MUS)_"},
    {ProjectIdentifier::regex_encounter_types,         "\\bTILE_ENCOUNTER_"},
    {ProjectIdentifier::regex_terrain_types,           "\\bTILE_TERRAIN_"},
    {ProjectIdentifier::regex_incbin,                  Regex::Pattern_INCBIN},
    // Other
    {ProjectIdentifier::pals_output_extension,         ".gbapal"},
    {ProjectIdentifier::tiles_output_extension,        ".4bpp.lz"},
};

const QMap<ProjectFilePath, QString> ProjectConfig::defaultPaths = {
    {ProjectFilePath::data_map_folders,                 "data/maps/"},
    {ProjectFilePath::data_scripts_folders,             "data/scripts/"},
    {ProjectFilePath::data_layouts_folders,             "data/layouts/"},
    {ProjectFilePath::data_primary_tilesets_folders,    "data/tilesets/primary/"},
    {ProjectFilePath::data_secondary_tilesets_folders,  "data/tilesets/secondary/"},
    {ProjectFilePath::data_event_scripts,               "data/event_scripts.s"},
    {ProjectFilePath::json_map_groups,                  "data/maps/map_groups.json"},
    {ProjectFilePath::json_layouts,                     "data/layouts/layouts.json"},
    {ProjectFilePath::json_wild_encounters,             "src/data/wild_encounters.json"},
    {ProjectFilePath::json_heal_locations,              "src/data/heal_locations.json"},
    {ProjectFilePath::json_region_map_entries,          "src/data/region_map/region_map_sections.json"},
    {ProjectFilePath::json_region_porymap_cfg,          "src/data/region_map/porymap_config.json"},
    {ProjectFilePath::tilesets_headers,                 "src/data/tilesets/headers.h"},
    {ProjectFilePath::tilesets_graphics,                "src/data/tilesets/graphics.h"},
    {ProjectFilePath::tilesets_metatiles,               "src/data/tilesets/metatiles.h"},
    {ProjectFilePath::tilesets_headers_asm,             "data/tilesets/headers.inc"},
    {ProjectFilePath::tilesets_graphics_asm,            "data/tilesets/graphics.inc"},
    {ProjectFilePath::tilesets_metatiles_asm,           "data/tilesets/metatiles.inc"},
    {ProjectFilePath::data_obj_event_gfx_pointers,      "src/data/object_events/object_event_graphics_info_pointers.h"},
    {ProjectFilePath::data_obj_event_gfx_info,          "src/data/object_events/object_event_graphics_info.h"},
    {ProjectFilePath::data_obj_event_pic_tables,        "src/data/object_events/object_event_pic_tables.h"},
    {ProjectFilePath::data_obj_event_gfx,               "src/data/object_events/object_event_graphics.h"},
    {ProjectFilePath::data_pokemon_gfx,                 "src/data/graphics/pokemon.h"},
    {ProjectFilePath::constants_global,                 "include/constants/global.h"},
    {ProjectFilePath::constants_items,                  "include/constants/items.h"},
    {ProjectFilePath::constants_flags,                  "include/constants/flags.h"},
    {ProjectFilePath::constants_vars,                   "include/constants/vars.h"},
    {ProjectFilePath::constants_weather,                "include/constants/weather.h"},
    {ProjectFilePath::constants_songs,                  "include/constants/songs.h"},
    {ProjectFilePath::constants_pokemon,                "include/constants/pokemon.h"},
    {ProjectFilePath::constants_map_types,              "include/constants/map_types.h"},
    {ProjectFilePath::constants_trainer_types,          "include/constants/trainer_types.h"},
    {ProjectFilePath::constants_secret_bases,           "include/constants/secret_bases.h"},
    {ProjectFilePath::constants_obj_event_movement,     "include/constants/event_object_movement.h"},
    {ProjectFilePath::constants_obj_events,             "include/constants/event_objects.h"},
    {ProjectFilePath::constants_event_bg,               "include/constants/event_bg.h"},
    {ProjectFilePath::constants_metatile_labels,        "include/constants/metatile_labels.h"},
    {ProjectFilePath::constants_metatile_behaviors,     "include/constants/metatile_behaviors.h"},
    {ProjectFilePath::constants_species,                "include/constants/species.h"},
    {ProjectFilePath::constants_fieldmap,               "include/fieldmap.h"},
    {ProjectFilePath::global_fieldmap,                  "include/global.fieldmap.h"},
    {ProjectFilePath::fieldmap,                         "src/fieldmap.c"},
    {ProjectFilePath::pokemon_icon_table,               "src/pokemon_icon.c"},
    {ProjectFilePath::initial_facing_table,             "src/event_object_movement.c"},
    {ProjectFilePath::wild_encounter,                   "src/wild_encounter.c"},
    {ProjectFilePath::pokemon_gfx,                      "graphics/pokemon/"},
};

std::optional<ProjectIdentifier> ProjectConfig::stringToProjectIdentifier(const QString& str) {
    return magic_enum::enum_cast<ProjectIdentifier>(str.toStdString(), magic_enum::case_insensitive);
}

std::optional<ProjectFilePath> ProjectConfig::stringToProjectFilePath(const QString& str) {
    return magic_enum::enum_cast<ProjectFilePath>(str.toStdString(), magic_enum::case_insensitive);
}

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
    }
    this->mapAllowFlagsEnabled = (this->baseGameVersion != BaseGame::Version::pokeruby);
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

    // Enforce this setting for projectConfig's plugins
    for (auto& settings : this->plugins) settings.userOnly = false;
}

QJsonObject ProjectConfig::getDefaultJson() const {
    ProjectConfig defaultConfig(this->baseGameVersion);
    // The defaults are version-specific, make sure we always output non-empty versions.
    defaultConfig.baseGameVersion = BaseGame::Version::none;
    return defaultConfig.toJson();
}

// TODO: Replace with a new prompt that allows choosing either the defaults for each version, or customizing settings.
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

        auto comboBox = new QComboBox();
        comboBox->addItem(BaseGame::versionToString(BaseGame::Version::pokeruby),    BaseGame::Version::pokeruby);
        comboBox->addItem(BaseGame::versionToString(BaseGame::Version::pokefirered), BaseGame::Version::pokefirered);
        comboBox->addItem(BaseGame::versionToString(BaseGame::Version::pokeemerald), BaseGame::Version::pokeemerald);
        form.addRow(new QLabel("Game Version"), comboBox);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
        QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        form.addRow(&buttonBox);

        if (dialog.exec() == QDialog::Accepted) {
            this->baseGameVersion = static_cast<BaseGame::Version>(comboBox->currentData().toInt());
        } else {
            logWarn(QString("No base_game_version selected, using default '%1'").arg(BaseGame::versionToString(this->baseGameVersion)));
        }
    }
    setVersionSpecificDefaults(this->baseGameVersion);
}

void ProjectConfig::setFilePath(ProjectFilePath pathId, const QString& path) {
    if (!defaultPaths.contains(pathId)) return;
    if (path.isEmpty()) {
        this->filePaths.remove(pathId);
    } else {
        this->filePaths[pathId] = path;
    }
}

void ProjectConfig::setFilePath(const QString& pathIdString, const QString& path) {
    std::optional<ProjectFilePath> pathId = stringToProjectFilePath(pathIdString);
    if (pathId.has_value()) setFilePath(pathId.value(), path);
}

QString ProjectConfig::getCustomFilePath(ProjectFilePath pathId) const {
    return QDir::cleanPath(this->filePaths.value(pathId));
}

QString ProjectConfig::getCustomFilePath(const QString& pathIdString) const {
    std::optional<ProjectFilePath> pathId = stringToProjectFilePath(pathIdString);
    return (pathId.has_value()) ? getCustomFilePath(pathId.value()) : QString();
}

QString ProjectConfig::getFilePath(ProjectFilePath pathId) const {
    QString customPath = getCustomFilePath(pathId);
    if (!customPath.isEmpty()) {
        // A custom filepath has been specified. If the file/folder exists, use that.
        const QString baseDir = projectDir() + "/";
        if (customPath.startsWith(baseDir)) {
            customPath.remove(0, baseDir.length());
        }
        if (QFileInfo::exists(QDir::cleanPath(baseDir + customPath))) {
            return customPath;
        } else {
            logError(QString("Custom project filepath '%1' not found. Using default.").arg(customPath));
        }
    }
    return defaultPaths.value(pathId);
}

void ProjectConfig::setIdentifier(ProjectIdentifier id, const QString& text) {
    if (!defaultIdentifiers.contains(id))
        return;

    if (text.isEmpty()) {
        this->identifiers.remove(id);
    } else {
        const QString idName = Converter<ProjectIdentifier>::toString(id);
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

void ProjectConfig::setIdentifier(const QString& idString, const QString& text) {
    std::optional<ProjectIdentifier> id = stringToProjectIdentifier(idString);
    if (id.has_value()) setIdentifier(id.value(), text);
}

QString ProjectConfig::getCustomIdentifier(ProjectIdentifier id) const {
    return this->identifiers.value(id);
}

QString ProjectConfig::getCustomIdentifier(const QString& idString) const {
    std::optional<ProjectIdentifier> id = stringToProjectIdentifier(idString);
    return (id.has_value()) ? getCustomIdentifier(id.value()) : QString();
}

QString ProjectConfig::getIdentifier(ProjectIdentifier id) const {
    const QString customText = getCustomIdentifier(id);
    return (!customText.isEmpty()) ? customText : defaultIdentifiers.value(id);
}
