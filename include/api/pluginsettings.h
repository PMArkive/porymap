#pragma once
#ifndef PLUGINSETTINGS_H
#define PLUGINSETTINGS_H

#include <QString>
#include <QList>

// Holds the basic user-provided information about a plugin.
struct PluginSettings {
    QString path;
    bool enabled = true;

    // Plugins can either be specific to the project, or specific to the user.
    // This allows projects to send plugin scripts downstream to their users,
    // while still allowing them to use their own personal plugins.
    bool userOnly = true;

    static QStringList filter(const QList<PluginSettings>& plugins) {
        QStringList paths;
        for (auto& plugin : plugins) {
            if (plugin.enabled) paths.append(plugin.path);
        }
        return paths;
    }
};

#endif // PLUGINSETTINGS_H
