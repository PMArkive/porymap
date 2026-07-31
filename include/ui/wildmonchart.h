#ifndef WILDMONCHART_H
#define WILDMONCHART_H

#include "encountertablemodel.h"

#include <QWidget>

class Project;

#if __has_include(<QtCharts>)
#include <QtCharts>

namespace Ui {
class WildMonChart;
}

class WildMonChart : public QWidget
{
    Q_OBJECT
public:
    explicit WildMonChart(QWidget *parent, const EncounterTableModel *table, Project *project);
    ~WildMonChart();

public slots:
    void setTable(const EncounterTableModel *table);
    void clearTable();
    void refresh();

private:
    Ui::WildMonChart *ui;
    const EncounterTableModel *table;
    Project *project = nullptr;

    QStringList groupNames;
    QStringList groupNamesReversed;
    QStringList speciesInLegendOrder;
    QMap<int, QString> tableIndexToGroupName;

    struct LevelRange {
        int min;
        int max;
    };
    QMap<QString, LevelRange> groupedLevelRanges;

    struct Summary {
        double speciesFrequency = 0.0;
        QMap<int, double> levelFrequencies;
    };
    typedef QMap<QString, Summary> GroupedData;

    QMap<QString, GroupedData> speciesToGroupedData;

    QStringList getSpeciesNamesAlphabetical() const;
    double getSpeciesFrequency(const QString&, const QString&) const;
    QMap<int, double> getLevelFrequencies(const QString &, const QString &) const;
    LevelRange getLevelRange(const QString &, const QString &) const;
    bool usesGroupLabels() const;

    void clearTableData();
    void readTable();
    QChart* createSpeciesDistributionChart();
    QChart* createLevelDistributionChart();
    QBarSet* createLevelDistributionBarSet(const QString &, const QString &, bool);
    void refreshSpeciesDistributionChart();
    void refreshLevelDistributionChart();

    QChart::ChartTheme currentTheme() const;
    void updateTheme();
    void limitChartAnimation();

    void showHelpDialog();
};

#else

class WildMonChart : public QWidget
{
    Q_OBJECT
public:
    explicit WildMonChart(QWidget *, const EncounterTableModel *, Project *) {};
    ~WildMonChart() {};

public slots:
    void setTable(const EncounterTableModel *) {};
    void clearTable() {};
    void refresh() {};
};

#endif // __has_include(<QtCharts>)

#endif // WILDMONCHART_H
