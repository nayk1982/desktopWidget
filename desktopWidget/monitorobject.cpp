#include "FileSys"
#include "Convert"
#include "HardwareUtils"
#include "Graph"
#include "SystemUtils"

#include "monitorobject.h"
//==============================================================================

using namespace nayk;
using namespace file_sys;

//==============================================================================
MonitorObject::MonitorObject(Settings *settings)
    : m_settings {settings}
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations);

    m_monVec = m_settings->monVec();
    m_boundingRect.setRect( 0, 0,
                            m_settings->screenGeometry().width(),
                            m_settings->box().height + 3 * m_settings->box().border_width);

    connect(&m_timer, &QTimer::timeout, this, &MonitorObject::on_timerTimeOut);
    m_timer.setSingleShot(false);
    m_timer.start(1000);
    m_busy = false;
}
//==============================================================================
MonitorObject::~MonitorObject()
{
    emit toLog(tr("MonitorObject Destructor"), Log::LogDbg);
}
//==============================================================================
QRectF MonitorObject::boundingRect() const
{
    return m_boundingRect;
}
//==============================================================================
void MonitorObject::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    for(const MonitorStruct &monitor: m_monVec)
        drawMonitor(painter, monitor);
}
//==============================================================================
void MonitorObject::updateValues()
{
    qreal rx = 0;
    qreal tx = 0;
    bool netOK = false;
    bool sec5 = m_counter % 5 == 0;
    bool sec10 = m_counter % 10 == 0;

    for(MonitorStruct &monitor: m_monVec) {

        qreal val_graph = 0.0;

        if(monitor.name == "cmd") {

            QString outStr;
            if(!system_utils::osCmd(monitor.arg, outStr, 600)) outStr = "0";
            bool ok;
            monitor.value = convert::strToDouble(outStr, &ok);
            if(!ok) monitor.value = 0;
            val_graph = monitor.value;
            for(SubStruct &sub: monitor.sub) {
                if(!system_utils::osCmd(sub.arg, outStr, 600)) outStr = "0";
                sub.value = outStr;
            }
        }
        else if(monitor.name == "cpu") {

            static QVector<qreal> vec;
            hardware_utils::cpuLoadPercent( vec );
            monitor.value = (vec.size() > 0) ? vec.at(0) : 0.0;
            for(auto j=0; j<monitor.sub.size(); ++j) {
                monitor.sub[j].value = ((j+1 < vec.size())
                                        ? convert::doubleToStr(vec.at(j+1), 2)
                                        : "0.00") + " %";
            }
        }
        else if((monitor.name == "mem") && sec5) {

            quint64 memTotal, memFree, memAvailable, swapTotal, swapFree;
            hardware_utils::memInfo(memTotal, memFree, memAvailable, swapTotal, swapFree);
            monitor.value = static_cast<qreal>(memTotal - memAvailable);
            monitor.max = static_cast<qreal>(memTotal);
            monitor.sub[0].value = getValueWithEd(memTotal);
            monitor.sub[1].value = getValueWithEd(memAvailable);
            monitor.sub[2].value = getValueWithEd(swapTotal);
            monitor.sub[3].value = getValueWithEd(swapFree);
        }
        else if((monitor.name == "temp") && sec10) {

            QString outStr;
            qreal val = 0.0;
            bool ok;

#if defined (Q_OS_WIN32)
            ok = hardware_utils::getTemperature( monitor.arg.toInt(), val );
            monitor.value = ok ? val : 0.0;
#else
            ok = system_utils::osCmd(monitor.arg, outStr, 600);
            if(ok) {
                val = convert::strToDouble(outStr, &ok);
                if(ok && (monitor.arg.indexOf("/hwmon/")>0)) val /= 1000.0;
            }
            monitor.value = ok ? val : 0.0;
#endif

            for(SubStruct &sub: monitor.sub) {

                val = 0.0;
#if defined (Q_OS_WIN32)
                ok = hardware_utils::getTemperature( sub.arg.toInt(), val );
#else
                ok = system_utils::osCmd(sub.arg, outStr, 600);
                if(ok) {
                    val = convert::strToDouble(outStr, &ok);
                    if(ok && (sub.arg.indexOf("/hwmon/")>0)) val /= 1000.0;
                }
#endif
                sub.value = (ok ? convert::doubleToStr(val, 2) : "0.00") + " °C";
            }
        }
        else if(monitor.name == "fs") {

            if(sec10) {
                quint64 sz, free;
                if(!hardware_utils::diskInfo( monitor.arg, sz, free )) {
                    sz = 0;
                    free = 0;
                }
                monitor.value = static_cast<qreal>(sz - free);
                monitor.max = static_cast<qreal>(sz);
                monitor.sub[0].value = getValueWithEd(sz);
                monitor.sub[1].value = getValueWithEd(free);
            }

            qreal io;
            if(hardware_utils::diskIOKB( monitor.dev, io )) {
                val_graph = m_counter == 0 ? 0 : io - monitor.graph_value;
                monitor.graph_value = io;
            }
            else {
                io = 0;
                val_graph = 0;
            }
        }
        else if((monitor.name == "up") || (monitor.name == "down")) {

            if(!netOK) {
#if defined (Q_OS_WIN32)
                QString err;
                netOK = hardware_utils::netInfo( monitor.arg, rx, tx, err );
                if(!netOK && isFirst)
                    emit toLog(err, Log::LogError);
#else
                netOK = hardware_utils::netInfo( monitor.arg, rx, tx );
#endif
            }

            qreal cnt = (monitor.name == "up") ? tx : rx;
            if(netOK) {
                val_graph = m_counter == 0 ? 0 : cnt - monitor.graph_value;
                monitor.graph_value = cnt;
            }
            else {
                cnt = 0.0;
                val_graph = 0.0;
            }
            monitor.value = val_graph;
            monitor.sub[0].value = getValueWithEd(val_graph);
            monitor.sub[1].value = getValueWithEd(cnt);
        }

        if(!monitor.graph.isEmpty()) {
            if (val_graph > monitor.graph_max ) monitor.graph_max = val_graph;
            for(auto j=0; j<monitor.graph.size()-1; ++j)
                monitor.graph[j] = monitor.graph[j+1];
            monitor.graph[monitor.graph.size()-1] = val_graph;
        }
    }
}
//==============================================================================
void MonitorObject::drawMonitor(QPainter *painter, const MonitorStruct &mon)
{
    graph::drawRectangle(painter, mon.x, mon.y, m_settings->box().width, m_settings->box().height,
                         m_settings->box().color, m_settings->box().border_color, m_settings->box().border_width,
                         m_settings->box().top_radius, m_settings->box().bot_radius
                         );

    graph::drawCircle(painter, mon.gauge_x, mon.gauge_y,
                      m_settings->gauge().ext_radius, QColor(0, 0, 0, 0),
                      m_settings->gauge().ext_border_color, m_settings->gauge().ext_border_width
                      );

    qreal value = qBound(0.0, mon.value, mon.max);
    QRectF gaugeRect( mon.gauge_x - m_settings->gauge().radius, mon.gauge_y - m_settings->gauge().radius,
                      2.0 * m_settings->gauge().radius, 2.0 * m_settings->gauge().radius
                      );

    qreal val_span_angle = value * m_settings->gauge().span_angle / mon.max;
    qreal angle = m_settings->gauge().mid_angle / 2.2;

    QRadialGradient bgGradient( mon.gauge_x, mon.gauge_y, m_settings->gauge().radius );
    QRadialGradient valGradient( mon.gauge_x, mon.gauge_y, m_settings->gauge().radius );

    qreal dd = 1.0 / m_settings->gauge().radius;

    bgGradient.setColorAt(0.0, QColor(0,0,0,0));
    bgGradient.setColorAt(dd * (m_settings->gauge().radius - m_settings->gauge().thickness) - 0.1, QColor(0,0,0,0));
    bgGradient.setColorAt(dd * (m_settings->gauge().radius - m_settings->gauge().thickness), m_settings->gauge().bg_color);
    bgGradient.setColorAt(1.0, m_settings->gauge().bg_color);

    valGradient.setColorAt(0.0, QColor(0,0,0,0));
    valGradient.setColorAt(dd * (m_settings->gauge().radius - m_settings->gauge().thickness) - 0.1, QColor(0,0,0,0));
    valGradient.setColorAt(dd * (m_settings->gauge().radius - m_settings->gauge().thickness), m_settings->gauge().color);
    valGradient.setColorAt(1.0, m_settings->gauge().color);

    painter->setPen( QPen(QBrush(QColor(0,0,0,0)), 0) );

    while(angle < m_settings->gauge().span_angle ) {

        if(angle > val_span_angle) painter->setBrush( QBrush( bgGradient ) );
        else painter->setBrush( QBrush( valGradient ) );

        painter->drawPie( gaugeRect, static_cast<int>(16*(270-angle)), static_cast<int>(-16 * m_settings->gauge().thick_angle) );
        painter->drawPie( gaugeRect, static_cast<int>(16*(270+angle)), static_cast<int>(16 * m_settings->gauge().thick_angle) );
        angle += m_settings->gauge().thick_angle + m_settings->gauge().mid_angle;
    }

    //
    graph::drawCircle(painter, mon.gauge_x, mon.title_y,
                      m_settings->gauge().title_radius, m_settings->gauge().title_bg_color,
                      m_settings->gauge().title_border_color, m_settings->gauge().title_border_width);

    drawText( painter, mon.gauge_x, mon.title_y - 1.0,
              mon.title, m_settings->gauge().titleFont, Qt::AlignCenter, false );

    //--- sub
    for(auto i=0; i<mon.sub.size(); ++i) {

        graph::drawCircle(painter, mon.bul_x, mon.y + mon.sub.at(i).y, m_settings->bullet().ext_radius,
                          QColor(0,0,0,0), m_settings->bullet().color, m_settings->bullet().ext_border_width );
        graph::drawCircle(painter, mon.bul_x, mon.y + mon.sub.at(i).y,
                          m_settings->bullet().radius, m_settings->bullet().color, m_settings->bullet().color, 0 );
        drawText(painter, mon.caption_x, mon.y + mon.sub.at(i).y,
                 mon.sub.at(i).title, m_settings->box().captionFont, Qt::AlignVCenter );
        drawText(painter, mon.value_x, mon.y + mon.sub.at(i).y,
                 mon.sub.at(i).value, m_settings->box().valueFont, Qt::AlignVCenter | Qt::AlignRight );
    }

    //--- graph
    if(mon.graph.isEmpty()) return;

    graph::drawRectangle(painter, mon.graph_x, mon.graph_y, m_settings->box().graph_width, m_settings->box().graph_height,
                         m_settings->graph().bg_color, m_settings->graph().border_color, m_settings->graph().border_width,
                         m_settings->graph().border_radius, m_settings->graph().border_radius );

    drawText(painter, mon.graph_x + m_settings->box().graph_width/2.0,
             mon.graph_y + m_settings->box().graph_height/2.0, mon.dev, m_settings->graph().watermarkFont, Qt::AlignCenter, false );

    qreal pixVal = (m_settings->box().graph_height-4) / (qFuzzyIsNull(mon.graph_max) ? 100.0 : mon.graph_max);

    QPainterPath path;
    path.moveTo( mon.graph_x + 3, mon.graph_y + m_settings->box().graph_height - 2 );
    int x = 0;
    while(x < mon.graph.size()) {
        path.lineTo( mon.graph_x + 3 + x, mon.graph_y + m_settings->box().graph_height - 2 - pixVal * mon.graph.at(x) );
        x++;
    }
    path.lineTo( mon.graph_x + 2 + x, mon.graph_y + m_settings->box().graph_height - 2 );
    path.closeSubpath();

    QLinearGradient lGrad( mon.graph_x, mon.graph_y + m_settings->box().graph_height - 2,
                           mon.graph_x, mon.graph_y + 2 );
    lGrad.setColorAt(0, m_settings->graph().colorStartGradient );
    lGrad.setColorAt(1, m_settings->graph().colorEndGradient );

    painter->setBrush( QBrush( lGrad ) );
    painter->fillPath(path, QBrush(lGrad));
    painter->drawPath(path);
}
//==============================================================================
void MonitorObject::drawText(QPainter *painter, qreal x, qreal y, const QString &text,
                             const FontStruct &font, Qt::Alignment align, bool withShadow)
{
    graph::drawText(painter, x, y, text, font.font_face, font.size, font.weight,
                    font.color, align,
                    withShadow ? m_settings->shadow().color : QColor(0,0,0,0),
                    m_settings->shadow().cx, m_settings->shadow().cy );
}
//==============================================================================
void MonitorObject::on_timerTimeOut()
{
    if(m_busy) return;
    m_busy = true;
    updateValues();
    if(++m_counter == 0) m_counter = 1;
    m_busy = false;
    update();
}
//==============================================================================
