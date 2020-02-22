#include <QImage>
#include "nayk/Graph"
#include "worldmapobject.h"
//==============================================================================

using namespace nayk;

//==============================================================================
WorldMapObject::WorldMapObject(Settings *settings)
    : m_settings {settings}
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations);

    m_curLoc = settings->currentLocation();
    m_boundingRect.setRect( 0, 0,
                            m_settings->bg().rect.width(),
                            m_settings->bg().rect.height() );
}
//==============================================================================
WorldMapObject::~WorldMapObject()
{
    emit toLog(tr("WorldMapObject Destructor"), Log::LogDbg);
}
//==============================================================================
void WorldMapObject::currentLocationChanged(const CurrentLocationStruct &currentLocation)
{
    emit toLog(tr("WorldMapObject slot currentLocationChanged()"), Log::LogDbg);
    m_curLoc = currentLocation;
    update();
}
//==============================================================================
QRectF WorldMapObject::boundingRect() const
{
    return m_boundingRect;
}
//==============================================================================
void WorldMapObject::paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    drawBackground(painter);
    drawCities(painter);
    drawCurrentLocation(painter);
}
//==============================================================================
void WorldMapObject::drawText(QPainter *painter, qreal x, qreal y,
                              const QString &text, const FontStruct &font,
                              Qt::Alignment align, bool withShadow)
{
    graph::drawText(painter, x, y, text, font.font_face, font.size, font.weight,
                    font.color, align,
                    withShadow ? m_settings->shadow().color : QColor(0,0,0,0),
                    m_settings->shadow().cx, m_settings->shadow().cy );
}
//==============================================================================
void WorldMapObject::drawBackground(QPainter *painter)
{
    if(m_settings->bg().ok) graph::drawImage( painter,
                                              m_settings->bg().img,
                                              m_settings->bg().rect );
}
//==============================================================================
void WorldMapObject::drawCities(QPainter *painter)
{
    for(const auto &rec: m_settings->miniCityVec()) {

        if(rec.city != m_curLoc.city)
            graph::drawCircle(painter, rec.point, m_settings->map().hide_caption_dot_radius,
                              m_settings->map().hide_caption_dot_color, m_settings->map().city_border_color,
                              m_settings->map().city_dot_border );
    }

    for(const auto &rec: m_settings->cityVec()) {

        if(rec.city != m_curLoc.city) {
            graph::drawCircle(painter, rec.point, m_settings->map().city_dot_radius,
                              m_settings->map().city_dot_color, m_settings->map().city_border_color,
                              m_settings->map().city_dot_border);
            qreal x = rec.point.x() + (m_settings->map().city_dot_radius + 4.0);
            qreal y = rec.point.y() - (m_settings->map().city_dot_radius - 1.0);

            drawText(painter, x, y, rec.city, m_settings->map().cityFont, Qt::AlignBottom);
        }
    }
}
//==============================================================================
void WorldMapObject::drawCurrentLocation(QPainter *painter)
{
    QPen linePen( QBrush( m_settings->map().line_color ), m_settings->map().line_width);

    constexpr qreal cy = 50.0;
    constexpr qreal cx = 50.0;

    painter->setPen( linePen );
    painter->drawLine( QPointF(m_curLoc.point.x(), cy),
                       QPointF(m_curLoc.point.x(),
                               m_settings->screenGeometry().height()
                               - m_settings->box().height - cy ) );
    painter->drawLine( QPointF(cx, m_curLoc.point.y()),
                       QPointF( m_settings->screenGeometry().width() - cx, m_curLoc.point.y()) );

    qreal x = m_curLoc.point.x() + (m_settings->map().current_dot_radius + 5.0);
    qreal y = m_curLoc.point.y() - (m_settings->map().current_dot_radius - 1.0);

    drawText(painter, x, y, m_curLoc.city, m_settings->map().currentFont, Qt::AlignBottom);

    x = m_settings->screenGeometry().width() - 128.0;
    y = m_curLoc.point.y() + 14.0;

    drawText(painter, x, y,
             QString("%1\n%2\n%3\nlat = %4\nlon = %5")
             .arg(m_curLoc.ip)
             .arg(m_curLoc.country)
             .arg(m_curLoc.city)
             .arg(m_curLoc.lat)
             .arg(m_curLoc.lon), m_settings->map().infoFont);
}
//==============================================================================
