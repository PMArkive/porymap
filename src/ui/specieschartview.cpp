#include "specieschartview.h"

#include "project.h"

#include <QPainter>

SpeciesChartView::SpeciesChartView(QWidget *parent)
    : QChartView(parent)
{
}


void SpeciesChartView::setSpecies(const QStringList &species)
{
    m_speciesEntries.clear();

    const QString speciesPrefix = projectConfig.getIdentifier(ProjectIdentifier::define_species_prefix);

    QFontMetrics fm(font());

    m_maxTextWidth = 0;
    for (const QString &name : species)
    {
        m_maxTextWidth = std::max(m_maxTextWidth, fm.horizontalAdvance(name));

        QPixmap icon = m_project->getSpeciesIcon(speciesPrefix + name);

        m_speciesEntries.push_back({
            name,
            icon.scaled(
                IconSize,
                IconSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation)
        });
    }

    viewport()->update();
}

void SpeciesChartView::setProject(Project *project)
{
    m_project = project;
    viewport()->update();
}

void SpeciesChartView::paintEvent(QPaintEvent *event)
{
    QChartView::paintEvent(event);

    if (!chart() || !m_project || m_speciesEntries.isEmpty())
        return;

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setPen(chart()->titleBrush().color());

    const QRectF plot = chart()->plotArea();

    if (plot.height() <= 0)
        return;

    const QFontMetrics fm(painter.font());

    const double rowHeight = plot.height() / m_speciesEntries.size();
    const int labelWidth = m_maxTextWidth + IconSize + Spacing;
    const int textX = plot.left() - labelWidth - Padding;
    const int iconX = textX + m_maxTextWidth + Spacing;

    for (int i = 0; i < m_speciesEntries.size(); ++i)
    {
        const SpeciesEntry &entry = m_speciesEntries[i];

        // Center text vertically
        const double y = plot.top() + rowHeight * (i + 0.5);
        const QRect textRect(
            textX,
            static_cast<int>(y - rowHeight / 2.0),
            m_maxTextWidth,
            static_cast<int>(rowHeight)
        );

        painter.drawText(
            textRect,
            Qt::AlignRight | Qt::AlignVCenter,
            entry.name
        );

        // Align icon with text
        const int iconY = textRect.center().y() - IconSize / 2.0 + IconVerticalOffset;
        const QRect iconRect(
            iconX,
            iconY,
            IconSize,
            IconSize
        );

        painter.drawPixmap(iconRect, entry.icon);
    }
}
