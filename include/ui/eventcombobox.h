#ifndef EVENTCOMBOBOX_H
#define EVENTCOMBOBOX_H

#include "noscrollwidgets.h"

#include <QRegularExpression>

class EventComboBox : public NoScrollComboBox
{
    Q_OBJECT
public:
    explicit EventComboBox(QWidget *parent = nullptr) : NoScrollComboBox(parent) {
        // Make speed a priority when loading comboboxes.
        // Set combo boxes to smallest reasonable size.
        setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

        // In general, event combo boxes are always editable.
        setEditable(true);
    }
};

#endif // EVENTCOMBOBOX_H
