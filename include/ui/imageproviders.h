#ifndef IMAGEPROVIDERS_H
#define IMAGEPROVIDERS_H

#include "block.h"
#include "tileset.h"
#include <QImage>
#include <QPixmap>

class Layout;

QImage getCollisionMetatileImage(Block);
QImage getCollisionMetatileImage(int, int);

QImage getMetatileImage(uint16_t, const Layout*, bool useTruePalettes = false);
QImage getMetatileImage(const Metatile*, const Layout*, bool useTruePalettes = false);
QImage getMetatileImage(uint16_t, const Tileset*, const Tileset*, const QList<int>& = {0,1,2}, const QList<float>& = {}, bool useTruePalettes = false);
QImage getMetatileImage(const Metatile*, const Tileset*, const Tileset*, const QList<int>& = {0,1,2}, const QList<float>& = {}, bool useTruePalettes = false);

QImage getMetatileSheetImage(const Layout *layout, int numMetatilesWIde, bool useTruePalettes = false);
QImage getMetatileSheetImage(const Tileset *primaryTileset,
                             const Tileset *secondaryTileset,
                             uint16_t metatileIdStart,
                             uint16_t metatileIdEnd,
                             int numMetatilesWIde,
                             const QList<int> &layerOrder,
                             const QList<float> &layerOpacity = {},
                             const QSize &metatileSize = Metatile::pixelSize(),
                             bool useTruePalettes = false);
QImage getMetatileSheetImage(const Tileset *primaryTileset,
                             const Tileset *secondaryTileset,
                             int numMetatilesWide,
                             const QList<int> &layerOrder,
                             const QList<float> &layerOpacity = {},
                             const QSize &metatileSize = Metatile::pixelSize(),
                             bool useTruePalettes = false);


QImage getTileImage(uint16_t, const Tileset*, const Tileset*);
QImage getPalettedTileImage(uint16_t, const Tileset*, const Tileset*, int, bool useTruePalettes = false);
QImage getColoredTileImage(uint16_t tileId, const Tileset *, const Tileset *, const QList<QRgb> &palette);
QImage getGreyscaleTileImage(uint16_t tileId, const Tileset *, const Tileset *);
const QList<QRgb>& greyscalePalette();

void flattenTo4bppImage(QImage * image);

#endif // IMAGEPROVIDERS_H
