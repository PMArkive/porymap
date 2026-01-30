#pragma once
#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <QStringList>
#include <QStack>
#include <QFileInfo>
#include "scriptutility.h"
#include "utility.h"

class Block;
class Tile;
class MainWindow;

#if __has_include(<QJSEngine>)
#include <QJSEngine>
#endif


#ifdef QT_QML_LIB

// !! New callback functions or changes to existing callback function names/arguments
//    should be synced to resources/text/script_template.txt and docsrc/manual/scripting-capabilities.rst
enum CallbackType {
    OnProjectOpened,
    OnProjectClosed,
    OnBlockChanged,
    OnBorderMetatileChanged,
    OnBlockHoverChanged,
    OnBlockHoverCleared,
    OnMapOpened,
    OnLayoutOpened,
    OnMapResized,
    OnBorderResized,
    OnMapShifted,
    OnTilesetUpdated,
    OnMainTabChanged,
    OnMapViewTabChanged,
    OnBorderVisibilityToggled,
    OnEventSpriteLoading,
};

class Scripting
{
public:
    Scripting(MainWindow *mainWindow);
    ~Scripting();
    static void init(MainWindow *mainWindow);
    static void stop();
    static void populateGlobalObject(MainWindow *mainWindow);
    static QJSEngine *getEngine();
    static QString getCurrentScriptHash();
    static void invokeAction(int actionIndex);
    static void setTimeout(QJSValue callback, int milliseconds);

    static void cb_ProjectOpened(QString projectPath);
    static void cb_ProjectClosed(QString projectPath);
    static void cb_MetatileChanged(int x, int y, Block prevBlock, Block newBlock);
    static void cb_BorderMetatileChanged(int x, int y, uint16_t prevMetatileId, uint16_t newMetatileId);
    static void cb_BlockHoverChanged(int x, int y);
    static void cb_BlockHoverCleared();
    static void cb_MapOpened(QString mapName);
    static void cb_LayoutOpened(QString layoutName);
    static void cb_MapResized(int oldWidth, int oldHeight, const QMargins &delta);
    static void cb_BorderResized(int oldWidth, int oldHeight, int newWidth, int newHeight);
    static void cb_MapShifted(int xDelta, int yDelta);
    static void cb_TilesetUpdated(const QString &tilesetName);
    static void cb_MainTabChanged(int oldTab, int newTab);
    static void cb_MapViewTabChanged(int oldTab, int newTab);
    static void cb_BorderVisibilityToggled(bool visible);
    static QImage cb_EventSpriteLoading(const QString &gfxName, const QString &direction);

    static bool tryErrorJS(QJSValue js);
    static QJSValue fromBlock(Block block);
    static QJSValue fromTile(Tile tile);
    static Tile toTile(QJSValue obj);
    static QJSValue dimensions(int width, int height);
    static QJSValue margins(const QMargins &margins);
    static QJSValue position(int x, int y);
    static const QImage * getImage(const QString &filepath, bool useCache);
    static QJSValue dialogInput(QJSValue input, bool selectedOk);
    static QJSValue fileResponse(const QString &s, bool isError);
    static bool checkFilePermissions(const QString &filepath);

private:
    MainWindow *mainWindow;
    QJSEngine *engine;

    class Script
    {
    public:
        Script() {};

        QString filepath() const { return m_filepath; }
        QString fileName() const {
            QFileInfo fileInfo(m_filepath);
            return fileInfo.fileName();
        }
        void setFilepath(const QString &filepath) { m_filepath = filepath; }

        QJSValue module() const { return m_module; }
        void setModule(const QJSValue &module) { m_module = module; }

        QString hash() {
            if (m_hash.isEmpty()) {
                // We won't need to check whether most scripts are trusted,
                // so only calculate the hash when it's first requested.
                m_hash = Util::getFileHash(m_filepath);
            }
            return m_hash;
        }
    private:
        QString m_filepath;
        QJSValue m_module;
        QString m_hash;
    };
    QList<QSharedPointer<Script>> scripts;

    // Tracks the scripts that are currently being evaluated.
    // This is useful for identifying which script is responsible
    // for calling a function at any point in time.
    // Because executing a callback/action in one script may trigger
    // a callback/action in a different script we may need to keep
    // track of multiple scripts executing at once.
    QStack<QSharedPointer<Script>> scriptExecutionStack;

    QSet<QTimer *> activeTimers;
    QMap<QString, const QImage*> imageCache;
    ScriptUtility *scriptUtility;

    void loadScript(const QString &filepath);
    QJSValue invokeCallback(CallbackType type, const QJSValueList &args);
    QSharedPointer<Script> getActiveScript() const;
    QJSValue call(QSharedPointer<Script> script, QJSValue func, const QJSValueList &args = QJSValueList());
    bool askForTrust(QSharedPointer<Script> script, const QString &reason);
};

#else

class Scripting
{
public:
    Scripting(MainWindow *) {}
    ~Scripting() {}
    static void init(MainWindow *) {}
    static void stop() {}
    static void populateGlobalObject(MainWindow *) {}

    static void cb_ProjectOpened(QString) {};
    static void cb_ProjectClosed(QString) {};
    static void cb_MetatileChanged(int, int, Block, Block) {};
    static void cb_BorderMetatileChanged(int, int, uint16_t, uint16_t) {};
    static void cb_BlockHoverChanged(int, int) {};
    static void cb_BlockHoverCleared() {};
    static void cb_MapOpened(QString) {};
    static void cb_LayoutOpened(QString) {};
    static void cb_MapResized(int, int, const QMargins &) {};
    static void cb_BorderResized(int, int, int, int) {};
    static void cb_MapShifted(int, int) {};
    static void cb_TilesetUpdated(const QString &) {};
    static void cb_MainTabChanged(int, int) {};
    static void cb_MapViewTabChanged(int, int) {};
    static void cb_BorderVisibilityToggled(bool) {};
    static QImage cb_EventSpriteLoading(const QString &, const QString &) {};
};

#endif // QT_QML_LIB

#endif // SCRIPTING_H
