#pragma once
#ifndef GUARD_WILDMONINFO_H
#define GUARD_WILDMONINFO_H

#include <QString>
#include <QVector>
#include "orderedjson.h"

class WildPokemon {
public:
    WildPokemon();
    WildPokemon(int minLevel, int maxLevel, const QString &species);

    int minLevel;
    int maxLevel;
    QString species;
    OrderedJson::object customData;

    static WildPokemon fromJson(const OrderedJson::object &json);
    OrderedJson::object toJson() const;
};

// Represents a single set of encounters, e.g. all the "land" encounters for one map.
// This includes the list of WildPokemon that are possible and the rate at which encounters trigger.
struct WildEncounters {
    bool active = false;
    int encounterRate = 0;
    QVector<WildPokemon> wildPokemon;
    OrderedJson::object customData;

    static WildEncounters fromJson(const OrderedJson::object &json, int numEncountersExpected = 0);
    OrderedJson::object toJson() const;
};

struct WildPokemonHeader {
    OrderedMap<QString, WildEncounters> wildMons;
    OrderedJson::object customData;
};

struct EncounterField {
    QString name; // Ex: "fishing_mons"
    QVector<int> encounterRates;
    OrderedMap<QString, QVector<int>> groups; // Ex: "good_rod", {2, 3, 4}
    OrderedJson::object customData;

    static EncounterField fromJson(const OrderedJson::object &json);
    OrderedJson::object toJson() const;
};

typedef QVector<EncounterField> EncounterFields;

void setDefaultEncounterRate(QString fieldName, int rate);
WildEncounters getDefaultEncounters(const EncounterField &field);
QVector<double> getWildEncounterPercentages(const EncounterField &field);
void combineEncounters(WildEncounters &to, WildEncounters from);

#endif // GUARD_WILDMONINFO_H
