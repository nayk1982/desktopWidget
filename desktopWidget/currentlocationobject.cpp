#include "nayk/NetworkClient"
#include "nayk/SystemUtils"
#include "nayk/Geo"
#include "nayk/Graph"
//
#include "currentlocationobject.h"
//==============================================================================

using namespace nayk;

//==============================================================================
CurrentLocationObject::CurrentLocationObject(Settings *settings)
    : m_settings {settings}
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations);

    m_curLoc = m_settings->currentLocation();
    qreal w = 2 * (m_settings->map().current_dot_radius + m_settings->map().current_dot_border);
    m_boundingRect.setRect( 0, 0, w, w );

    connect(&m_timer, &QTimer::timeout, this, &CurrentLocationObject::on_timerTimeOut);
    m_timer.setSingleShot(false);
    m_timer.start(1000);
}
//==============================================================================
CurrentLocationObject::~CurrentLocationObject()
{
    emit toLog(tr("CurrentLocationObject Destructor"), Log::LogDbg);
}
//==============================================================================
QRectF CurrentLocationObject::boundingRect() const
{
    return m_boundingRect;
}
//==============================================================================
void CurrentLocationObject::paint(QPainter *painter,
                                  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    graph::drawCircle( painter, m_boundingRect.center(), m_settings->map().current_dot_radius,
                           (m_altDotColor ? m_settings->map().current_alt_color : m_settings->map().current_dot_color),
                           m_settings->map().current_border_color, m_settings->map().current_dot_border
                           );
}
//==============================================================================
void CurrentLocationObject::updateCurrentLocation()
{
    emit toLog(tr("Update current location"), Log::LogInfo);

    m_curLoc.ip = "";
    QString par = "?access_key=" + m_settings->common().api_key +
            "&language=ru&output=json&fields=ip,city,latitude,longitude,country_name";
    QString url = "http://api.ipapi.com/";

    if(network_client::externalIP(m_curLoc.ip)) {

        url += m_curLoc.ip + par;
    }
    else {

        url += "api/check" + par;
        m_curLoc.ip = "";
    }

    m_curLoc.point = QPointF(0, 0);

    QString jsonStr;

    emit toLog(url, Log::LogOut);

    if(network_client::simpleGet(url, jsonStr)) {

        emit toLog(jsonStr, Log::LogIn);

        QJsonObject obj = QJsonDocument::fromJson( jsonStr.toUtf8() ).object();

        m_curLoc.lat = obj.value("latitude").toDouble();
        m_curLoc.lon = obj.value("longitude").toDouble();
        m_curLoc.city = obj.value("city").toString("0,0");
        m_curLoc.country = obj.value("country_name").toString("");
        m_curLoc.ip = obj.value("ip").toString(m_curLoc.ip);

    }
    else {

        emit toLog(jsonStr, Log::LogError);

        m_curLoc.lat = 0.0;
        m_curLoc.lon = 0.0;
        m_curLoc.city =  QString("0,0");
        m_curLoc.country = QString("");
    }

    m_curLoc.point = geo::coordGeoToMap( m_curLoc.lat, m_curLoc.lon,
                                         m_settings->map().map_width,
                                         m_settings->map().map_cx,
                                         m_settings->map().map_cy );
}
//==============================================================================
void CurrentLocationObject::checkCurrentLocation()
{
    emit toLog(tr("Check current location in cities list"), Log::LogInfo);

    for(const CityStruct &city: m_settings->cityVec()) {

        if(isEqualPoints(city.point, m_curLoc.point)) {

            m_curLoc.point = city.point;
            m_curLoc.city = city.city;
            return;
        }
    }

    for(const CityStruct &city: m_settings->miniCityVec()) {

        if(isEqualPoints(city.point, m_curLoc.point)) {

            m_curLoc.point = city.point;
            m_curLoc.city = city.city;
            return;
        }
    }
}
//==============================================================================
void CurrentLocationObject::on_timerTimeOut()
{
    if(++m_counter % m_settings->common().update_current_sec == 0) {

        updateCurrentLocation();
        checkCurrentLocation();
        emit locationChanged(m_curLoc);
    }

    m_altDotColor = !m_altDotColor;
    update();
}
//==============================================================================
