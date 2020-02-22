#ifndef WORLDMAPOBJECT_H
#define WORLDMAPOBJECT_H

#include <QGraphicsObject>
//
#include "nayk/Log"
//
#include "common.h"
#include "settings.h"
//==============================================================================
class WorldMapObject : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit WorldMapObject(Settings *settings);
    virtual ~WorldMapObject();

signals:
    void toLog(const QString &text, nayk::Log::LogType logType = nayk::Log::LogInfo);

public slots:
    void currentLocationChanged(const CurrentLocationStruct &currentLocation);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    Settings *m_settings;
    QRectF m_boundingRect;
    CurrentLocationStruct m_curLoc;

    void drawText(QPainter* painter, qreal x, qreal y, const QString &text, const FontStruct &font,
                          Qt::Alignment align = Qt::AlignLeft | Qt::AlignTop, bool withShadow = true);
    void drawBackground(QPainter* painter);
    void drawCities(QPainter* painter);
    void drawCurrentLocation(QPainter* painter);
};
//==============================================================================
#endif // WORLDMAPOBJECT_H
