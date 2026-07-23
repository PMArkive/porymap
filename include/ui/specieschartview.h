#ifndef SPECIESCHARTVIEW_H
#define SPECIESCHARTVIEW_H

#if __has_include(<QtCharts>)
#include <QtCharts/QChartView>

class Project;

struct SpeciesEntry
{
    QString name;
    QPixmap icon;
};

class SpeciesChartView : public QChartView
{
    Q_OBJECT

public:
    static constexpr int IconSize = 32;
    static constexpr int IconVerticalOffset = -2;
    static constexpr int Spacing = 6;
    static constexpr int Padding = 10;
    explicit SpeciesChartView(QWidget *parent = nullptr);

    void setSpecies(const QStringList &species);
    void setProject(Project *project);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<SpeciesEntry> m_speciesEntries;
    Project *m_project = nullptr;
    int m_maxTextWidth = 0;
};

#endif // __has_include(<QtCharts>)

#endif // SPECIESCHARTVIEW_H
