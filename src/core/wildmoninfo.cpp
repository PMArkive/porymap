#include "wildmoninfo.h"
#include "montabwidget.h"
#include "project.h"

WildPokemon::WildPokemon(int minLevel, int maxLevel, const QString &species)
    : minLevel(minLevel),
      maxLevel(maxLevel),
      species(species)
{}

WildPokemon::WildPokemon() : WildPokemon(5, 5, Project::getEmptySpeciesName())
{}

WildPokemon WildPokemon::fromJson(const OrderedJson::object &json_) {
    OrderedJson::object json(json_);

    WildPokemon wildMon;
    wildMon.minLevel = json.take("min_level").toInt();
    wildMon.maxLevel = json.take("max_level").toInt();
    wildMon.species = json.take("species").toString();
    wildMon.customData = json;
    return wildMon;
}

OrderedJson::object WildPokemon::toJson() const {
    OrderedJson::object json;
    json["min_level"] = this->minLevel;
    json["max_level"] = this->maxLevel;
    json["species"] = this->species;
    OrderedJson::append(&json, this->customData);
    return json;
}

WildEncounters WildEncounters::fromJson(const OrderedJson::object &json_, int numEncountersExpected) {
    OrderedJson::object json(json_);

    WildEncounters encounters;
    encounters.active = true;
    encounters.encounterRate = json.take("encounter_rate").toInt();

    // Read wild pokémon list
    OrderedJson::array monsArray = json.take("mons").array_items();
    for (const auto &monJson : monsArray) {
        auto wildMon = WildPokemon::fromJson(monJson.object_items());
        encounters.wildPokemon.append(wildMon);
    }
    encounters.customData = json;

    // If the user supplied too few pokémon for this group then we fill in the rest with default values.
    if (numEncountersExpected > 0) {
        for (int i = encounters.wildPokemon.length(); i < numEncountersExpected; i++) {
            encounters.wildPokemon.append(WildPokemon());
        }
    }
    return encounters;
}

OrderedJson::object WildEncounters::toJson() const {
    OrderedJson::object json;
    json["encounter_rate"] = this->encounterRate;
    OrderedJson::array monArray;
    for (const WildPokemon &wildMon : this->wildPokemon) {
        monArray.push_back(wildMon.toJson());
    }
    json["mons"] = monArray;
    OrderedJson::append(&json, this->customData);
    return json;
}

EncounterField EncounterField::fromJson(const OrderedJson::object &json_) {
    OrderedJson::object json(json_);

    EncounterField field;
    field.name = json.take("type").toString();

    OrderedJson::array encounterRatesArray = json.take("encounter_rates").array_items();
    for (const auto &val : encounterRatesArray) {
        field.encounterRates.append(val.toInt());
    }

    // Each element of the "groups" array is an object with the group name as the key (e.g. "old_rod")
    // and an array of slot numbers indicating which encounter slots in this encounter type belong to that group.
    OrderedJson::object groups = json.take("groups").object_items();
    for (auto groupPair : groups) {
        const QString groupName = groupPair.first;
        const OrderedJson::array slotNums = groupPair.second.array_items();
        for (auto slotNum : slotNums) {
            field.groups[groupName].append(slotNum.toInt());
        }
    }
    field.customData = json;

    return field;
}

OrderedJson::object EncounterField::toJson() const {
    OrderedJson::object json;

    json["type"] = this->name;

    OrderedJson::array rateArray;
    for (int rate : this->encounterRates) {
        rateArray.push_back(rate);
    }
    json["encounter_rates"] = rateArray;

    OrderedJson::object groupsObject;
    for (const auto &groupPair : this->groups) {
        QVector<int> slots_ = groupPair.second;
        OrderedJson::array slotsJson;
        std::sort(slots_.begin(), slots_.end());
        for (int slotIndex : slots_) {
            slotsJson.push_back(slotIndex);
        }
        const QString groupName = groupPair.first;
        groupsObject[groupName] = slotsJson;
    }
    if (!groupsObject.empty()) json["groups"] = groupsObject;

    OrderedJson::append(&json, this->customData);
    return json;
}

QMap<QString, int> defaultEncounterRates;
void setDefaultEncounterRate(QString fieldName, int rate) {
    defaultEncounterRates[fieldName] = rate;
}

WildEncounters getDefaultEncounters(const EncounterField &field) {
    WildEncounters encounters;
    encounters.active = true;
    encounters.encounterRate = defaultEncounterRates.value(field.name, 1);

    int size = field.encounterRates.size();
    while (size--)
        encounters.wildPokemon.append(WildPokemon());

    return encounters;
}

QVector<double> getWildEncounterPercentages(const EncounterField &field) {
    QVector<double> percentages(field.encounterRates.size(), 0);

    if (!field.groups.empty()) {
        // This encounter field is broken up into groups (e.g. for fishing rod types).
        // Each group's percentages will be relative to the group total, not the overall total.
        for (auto groupKeyPair : field.groups) {
            int groupTotal = 0;
            for (int slot : groupKeyPair.second) {
                groupTotal += field.encounterRates.value(slot, 0);
            }
            if (groupTotal != 0) {
                for (int slot : groupKeyPair.second) {
                    percentages[slot] = static_cast<double>(field.encounterRates.value(slot, 0)) / static_cast<double>(groupTotal);
                }
            }
        }
    } else {
        // This encounter field has a single group, percentages are relative to the overall total.
        int groupTotal = 0;
        for (int chance : field.encounterRates) {
            groupTotal += chance;
        }
        if (groupTotal != 0) {
            for (int slot = 0; slot < percentages.count(); slot++) {
                percentages[slot] = static_cast<double>(field.encounterRates.value(slot, 0)) / static_cast<double>(groupTotal);
            }
        }
    }
    return percentages;
}

void combineEncounters(WildEncounters &to, WildEncounters from) {
    to.encounterRate = from.encounterRate;

    if (to.wildPokemon.size() == from.wildPokemon.size()) {
        to.wildPokemon = from.wildPokemon;
    }
    else if (to.wildPokemon.size() > from.wildPokemon.size()) {
        to.wildPokemon = from.wildPokemon + to.wildPokemon.mid(from.wildPokemon.size());
    }
    else {
        to.wildPokemon = from.wildPokemon.mid(0, to.wildPokemon.size());
    }
}
