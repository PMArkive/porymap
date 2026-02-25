#pragma once
#ifndef SCRIPTSETTINGS_H
#define SCRIPTSETTINGS_H

#include <QString>
#include <QList>

// Holds the basic user-provided information about a plug-in script.
struct ScriptSettings {
    QString path;
    bool enabled = true;

    // Scripts can either be specific to the project, or specific to the user.
    // This allows projects to send scripts downstream to their users,
    // while still allowing them to use their own personal scripts.
    bool userOnly = true;

    static QStringList filter(const QList<ScriptSettings>& scripts) {
        QStringList paths;
        for (auto& script : scripts) {
            if (script.enabled) paths.append(script.path);
        }
        return paths;
    }
};

#endif // SCRIPTSETTINGS_H
