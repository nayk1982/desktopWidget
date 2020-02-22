#include "nayk/Graph"
#include "utctextobject.h"
//==============================================================================

using namespace nayk;

//==============================================================================
UtcTextObject::UtcTextObject(Settings *settings)
    : m_settings {settings}
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations);

    m_text = tr("%1 00:00\nlon = %2")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"))
            .arg(0.0);
    qreal fontSize = m_settings->map().infoFont.size + 4;
    m_boundingRect.setRect( 0, 0, 10 + fontSize * 16, fontSize * 3);
}
//==============================================================================
UtcTextObject::~UtcTextObject()
{
    emit toLog(tr("UtcTextObject Destructor"), Log::LogDbg);
}
//==============================================================================
void UtcTextObject::dataChanged(QDate date, qreal longitude)
{
    m_text = tr("%1 00:00\nlon = %2")
            .arg(date.toString("yyyy-MM-dd"))
            .arg(longitude);
    update();
}
//==============================================================================
QRectF UtcTextObject::boundingRect() const
{
    return m_boundingRect;
}
//==============================================================================
void UtcTextObject::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    drawText(painter, 0, 0,
             m_text, m_settings->map().infoFont, Qt::AlignLeft);
}
//==============================================================================
void UtcTextObject::drawText(QPainter *painter, qreal x, qreal y,
                             const QString &text, const FontStruct &font,
                             Qt::Alignment align, bool withShadow)
{
    graph::drawText(painter, x, y, text, font.font_face, font.size, font.weight,
                    font.color, align,
                    withShadow ? m_settings->shadow().color : QColor(0,0,0,0),
                    m_settings->shadow().cx, m_settings->shadow().cy );
}
//==============================================================================
