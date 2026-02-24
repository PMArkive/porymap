#pragma once
#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include "keyvalueconfigbase.h"
#include "block.h"
#include "events.h"

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

// Distance in pixels from the edge of a GBA screen (240x160) to the center 16x16 pixels.
#define GBA_H_DIST_TO_CENTER ((240-16)/2)
#define GBA_V_DIST_TO_CENTER ((160-16)/2)

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

    FieldManager* getFieldManager() override {
        if (!m_fm) {
            m_fm = std::make_shared<FieldManager>();
            m_fm->addField(&this->baseGameVersion, "base_game_version");
            m_fm->addField(&this->usePoryScript, "use_poryscript");
            m_fm->addField(&this->useCustomBorderSize, "use_custom_border_size");
            m_fm->addField(&this->eventWeatherTriggerEnabled, "enable_event_weather_trigger");
            m_fm->addField(&this->eventSecretBaseEnabled, "enable_event_secret_base");
            m_fm->addField(&this->hiddenItemQuantityEnabled, "enable_hidden_item_quantity");
            m_fm->addField(&this->hiddenItemRequiresItemfinderEnabled, "enable_hidden_item_requires_itemfinder");
            m_fm->addField(&this->healLocationRespawnDataEnabled, "enable_heal_location_respawn_data");
            m_fm->addField(&this->eventCloneObjectEnabled, "enable_event_clone_object");
            m_fm->addField(&this->floorNumberEnabled, "enable_floor_number");
            m_fm->addField(&this->createMapTextFileEnabled, "create_map_text_file");
            m_fm->addField(&this->tripleLayerMetatilesEnabled, "enable_triple_layer_metatiles");
            m_fm->addField<uint16_t>(&this->defaultMetatileId, "default_metatile_id", 0, Block::MaxValue);
            m_fm->addField<uint16_t>(&this->defaultElevation, "default_elevation", 0, Block::MaxValue);
            m_fm->addField<uint16_t>(&this->defaultCollision, "default_collision", 0, Block::MaxValue);
            m_fm->addField(&this->defaultMapSize, "default_map_size");
            m_fm->addField(&this->newMapBorderMetatileIds, "new_map_border_metatiles");
            m_fm->addField(&this->defaultPrimaryTileset, "default_primary_tileset");
            m_fm->addField(&this->defaultSecondaryTileset, "default_secondary_tileset");
            m_fm->addField(&this->tilesetsHaveCallback, "tilesets_have_callback");
            m_fm->addField(&this->tilesetsHaveIsCompressed, "tilesets_have_is_compressed");
            m_fm->addField(&this->transparencyColor, "transparency_color");
            m_fm->addField(&this->preserveMatchingOnlyData, "preserve_matching_only_data");
            m_fm->addField(&this->metatileAttributesSize, "metatile_attributes_size");
            m_fm->addField(&this->metatileBehaviorMask, "metatile_behavior_mask");
            m_fm->addField(&this->metatileTerrainTypeMask, "metatile_terrain_type_mask");
            m_fm->addField(&this->metatileEncounterTypeMask, "metatile_encounter_type_mask");
            m_fm->addField(&this->metatileLayerTypeMask, "metatile_layer_type_mask");
            m_fm->addField<uint16_t>(&this->blockMetatileIdMask, "block_metatile_id_mask", 0, Block::MaxValue);
            m_fm->addField<uint16_t>(&this->blockCollisionMask, "block_collision_mask", 0, Block::MaxValue);
            m_fm->addField<uint16_t>(&this->blockElevationMask, "block_elevation_mask", 0, Block::MaxValue);
            m_fm->addField<uint16_t>(&this->unusedTileNormal, "unused_tile_normal", 0, Tile::MaxValue);
            m_fm->addField<uint16_t>(&this->unusedTileCovered, "unused_tile_covered", 0, Tile::MaxValue);
            m_fm->addField<uint16_t>(&this->unusedTileSplit, "unused_tile_split", 0, Tile::MaxValue);
            m_fm->addField(&this->mapAllowFlagsEnabled, "enable_map_allow_flags");
            m_fm->addField(&this->eventsTabIconPath, "events_tab_icon_path");
            m_fm->addField(&this->collisionSheetPath, "collision_sheet_path");
            m_fm->addField(&this->collisionSheetSize, "collision_sheet_size", QSize(1,1), QSize(Block::MaxValue, Block::MaxValue));
            m_fm->addField(&this->playerViewDistance, "player_view_distance", QMargins(0,0,0,0), QMargins(INT_MAX, INT_MAX, INT_MAX, INT_MAX));
            m_fm->addField(&this->warpBehaviors, "warp_behaviors");
            m_fm->addField(&this->maxEventsPerGroup, "max_events_per_group", 1, INT_MAX);
            m_fm->addField(&this->metatileSelectorWidth, "metatile_selector_width", 1, INT_MAX);
            m_fm->addField(&this->globalConstantsFilepaths, "global_constants_filepaths");
            m_fm->addField(&this->globalConstants, "global_constants");
            m_fm->addField(&this->customScripts, "custom_scripts");
            m_fm->addField(&this->eventIconPaths, "event_icon_paths");
            m_fm->addField(&this->pokemonIconPaths, "pokemon_icon_paths");
            m_fm->addField(&this->minimumVersion, "minimum_version");

            m_fm->addField(&this->identifiers, "custom_identifiers");
            m_fm->addField(&this->filePaths, "custom_file_paths");
        }
        return m_fm.get();
    }

protected:
    virtual bool parseLegacyKeyValue(const QString& key, const QString& value) override;
    virtual QJsonObject getDefaultJson() const override;
    virtual void initializeFromEmpty() override;

private:
    ProjectFilePath reverseDefaultPaths(const QString& str);
    ProjectIdentifier reverseDefaultIdentifier(const QString& str);

    std::shared_ptr<FieldManager> m_fm = nullptr;
    QMap<ProjectIdentifier, QString> identifiers;
    QMap<ProjectFilePath, QString> filePaths;
};

extern ProjectConfig projectConfig;

#endif // PROJECTCONFIG_H
