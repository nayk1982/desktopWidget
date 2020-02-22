#include "nayk/Graph"
#include "utclineobject.h"
//==============================================================================

using namespace nayk;

//==============================================================================
UtcLineObject::UtcLineObject(Settings *settings)
    : m_settings {settings}
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations);

    m_boundingRect.setRect( 0, 0,
                            m_settings->utcLine().line_width * 2.0,
                            m_settings->screenGeometry().height()
                            - m_settings->box().height - 40.0 );
}
//==============================================================================
UtcLineObject::~UtcLineObject()
{
    emit toLog(tr("UtcLineObject Destructor"), Log::LogDbg);
}
//==============================================================================
QRectF UtcLineObject::boundingRect() const
{
    return m_boundingRect;
}
//==============================================================================
void UtcLineObject::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    qreal x = m_boundingRect.width() / 2.0;
    QPen linePen( QBrush( m_settings->utcLine().line_color ),
                  m_settings->utcLine().line_width, Qt::DashLine);
    painter->setPen( linePen );
    painter->drawLine( QPointF(x, 0),
                       QPointF(x, m_boundingRect.height()) );
}
//==============================================================================
