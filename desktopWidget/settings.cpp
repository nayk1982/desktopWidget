#include "QApplication"
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QScreen>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
//
#include "nayk/FileSys"
#include "nayk/Convert"
#include "nayk/AppCore"
#include "nayk/SystemUtils"
#include "nayk/Geo"
#include "nayk/Graph"
#include "nayk/NetworkClient"
//
#include "settings.h"
//==============================================================================

using namespace nayk;
using namespace file_sys;

//==============================================================================
Settings::Settings(QObject *parent) : QObject(parent)
{
    m_appDir = app_core::applicationFullPath();
    m_profileDir = app_core::applicationProfilePath();
    connect(&m_fsWatcher, &QFileSystemWatcher::fileChanged,
            this, &Settings::on_fileChanged);
}
//==============================================================================
Settings::~Settings()
{

}
//==============================================================================
QString Settings::appDir() const
{
    return m_appDir;
}
//==============================================================================
QString Settings::profileDir() const
{
    return m_profileDir;
}
//==============================================================================
int Settings::displayNum() const
{
    return m_displayNum;
}
//==============================================================================
bool Settings::debug() const
{
    return m_debug;
}
//==============================================================================
QRect Settings::screenGeometry() const
{
    return m_screenGeometry;
}
//==============================================================================
MapStruct Settings::map() const
{
    return m_map;
}
//==============================================================================
CommonStruct Settings::common() const
{
    return m_comm;
}
//==============================================================================
BgStruct Settings::bg() const
{
    return m_bg;
}
//==============================================================================
CurrentLocationStruct Settings::currentLocation() const
{
    return m_curLoc;
}
//==============================================================================
ShadowStruct Settings::shadow() const
{
    return m_shadow;
}
//==============================================================================
BoxStruct Settings::box() const
{
    return m_box;
}
//==============================================================================
GaugeStruct Settings::gauge() const
{
    return m_gauge;
}
//==============================================================================
BulletStruct Settings::bullet() const
{
    return m_bullet;
}
//==============================================================================
GraphStruct Settings::graph() const
{
    return m_graph;
}
//==============================================================================
UtcLineStruct Settings::utcLine() const
{
    return m_utcLine;
}
//==============================================================================
QVector<CityStruct> Settings::miniCityVec() const
{
    return m_miniCityVec;
}
//==============================================================================
QVector<CityStruct> Settings::cityVec() const
{
    return m_cityVec;
}
//==============================================================================
QVector<MonitorStruct> Settings::monVec() const
{
    return m_monVec;
}
//==============================================================================
bool Settings::mapVisible() const
{
    return m_mapVisible;
}
//==============================================================================
void Settings::readSettings()
{
    emit toLog(tr("Read settings"), Log::LogInfo);

    if(!m_fsWatcher.files().isEmpty())
            m_fsWatcher.removePaths( m_fsWatcher.files() );

    QSettings ini(m_profileDir + file_Config, QSettings::IniFormat);
    ini.setIniCodec("UTF-8");
    m_debug = ini.value("main/dbg", false).toBool();
    m_mapVisible = ini.value("main/map_visible", true).toBool();
    m_displayNum = qBound(0 ,
                          ini.value("main/display", 0).toInt(),
                          QGuiApplication::screens().size()-1
                          );
    QScreen *screen = QGuiApplication::screens().at( m_displayNum );
    m_screenGeometry = screen->availableGeometry();

    emit toLog(tr("display=%1, left=%2, top=%3, width=%4, height=%5, dbg=%6")
               .arg(m_displayNum)
               .arg(m_screenGeometry.left())
               .arg(m_screenGeometry.top())
               .arg(m_screenGeometry.width())
               .arg(m_screenGeometry.height())
               .arg(m_debug),
               Log::LogDbg);

    if(!fileExists(m_profileDir + file_Config)) {

        emit toLog(tr("Write settings to file: %1")
                   .arg(m_profileDir + file_Config), Log::LogDbg);

        ini.setValue("main/display", m_displayNum);
        ini.setValue("main/dbg", m_debug);
        ini.sync();
    }
    m_fsWatcher.addPath(m_profileDir + file_Config);

    if(fileExists(m_profileDir + file_Style)) {

        emit toLog(tr("Load stylesheet from file: %1")
                   .arg(m_profileDir + file_Style), Log::LogDbg);

        QFile file(m_profileDir + file_Style);
        if (file.open(QFile::ReadOnly)) {
            qApp->setStyleSheet( QString(file.readAll()) );
            file.close();
            m_fsWatcher.addPath(m_profileDir + file_Style);
        }
        else {
            emit toLog(tr("Failed open file: %1\n%2")
                       .arg(m_profileDir + file_Style)
                       .arg(file.errorString()), Log::LogError);
        }
    }
    else {
        qApp->setStyleSheet("Canvas { background: rgba(0,0,0,0); }");
    }

    QJsonObject commonObj;
    if(loadJson( m_profileDir + file_CommonJson,
                 ":/json/common.json", commonObj )) {
        m_fsWatcher.addPath(m_profileDir + file_CommonJson);
    }

    QJsonObject monitorObj;
    if(loadJson( m_profileDir + file_MonitorJson,
                 ":/json/monitor.json", monitorObj )) {
        m_fsWatcher.addPath(m_profileDir + file_MonitorJson);
    }

    updateCommonConfig(commonObj);

    if(!m_mapVisible)
        m_screenGeometry.setTop( m_screenGeometry.bottom() - qRound(m_box.height + 3.0 * m_box.border_width + 0.5) );

    updateMonitorConfig(monitorObj);

    if(m_mapVisible) {

        QJsonObject worldmapObj;
        if(loadJson( m_profileDir + file_WorldmapJson,
                     ":/json/worldmap.json", worldmapObj )) {
            m_fsWatcher.addPath(m_profileDir + file_WorldmapJson);
        }
        updateWorldmapConfig(worldmapObj);
    }
}
//==============================================================================
bool Settings::loadJson(const QString &fileName, const QString &resFileName,
                        QJsonObject &jsonObject)
{
    QString err;

    if(fileExists( fileName )) {

        if(readJsonFromFile(fileName, jsonObject, err)) {

            emit toLog(tr("Load json settings from file: %1")
                       .arg(fileName), Log::LogDbg);
        }
        else {
            emit toLog(tr("Failed open file: %1\n%2")
                       .arg(fileName)
                       .arg(err), Log::LogError);
            return false;
        }
    }
    else {

        if(readJsonFromFile(resFileName, jsonObject, err)) {

            emit toLog(tr("Load json settings from file: %1")
                       .arg(resFileName), Log::LogDbg);

            if(writeJsonToFile(fileName, jsonObject, err)) {
                emit toLog(tr("Write json settings to file: %1")
                           .arg(fileName), Log::LogDbg);
            }
            else {
                emit toLog(tr("Failed write file: %1\n%2")
                           .arg(fileName)
                           .arg(err), Log::LogError);
            }
        }
        else {
            emit toLog(tr("Failed open file: %1\n%2")
                       .arg(resFileName)
                       .arg(err), Log::LogError);
            return false;
        }
    }

    return true;
}
//==============================================================================
void Settings::updateCommonConfig(const QJsonObject &config)
{
    emit toLog(tr("Load common config from JSON"), Log::LogInfo);

    // shadow -------------------
    m_shadow.color = graph::colorFromArray( config.value("shadow").toObject().value("color").toArray() );
    m_shadow.cx = config.value("shadow").toObject().value("cx").toDouble();
    m_shadow.cy = config.value("shadow").toObject().value("cy").toDouble();

    m_comm.api_key = config.value("api_key").toString("");
    m_comm.api_username = config.value("api_username").toString("demo");
    m_comm.font_face = config.value("font_face").toString("Monospace");
    m_comm.update_current_sec = config.value("update_current_secs").toInt();

    // graphic -------------------
    m_graph.bg_color = graph::colorFromArray( config.value("graph").toObject()
                                              .value("bg_color").toArray() );
    m_graph.border_color = graph::colorFromArray( config.value("graph").toObject()
                                                  .value("border_color").toArray() );
    m_graph.border_width = config.value("graph").toObject().value("border_width").toDouble();
    m_graph.border_radius = config.value("graph").toObject().value("border_radius").toDouble();
    m_graph.colorStartGradient = graph::colorFromArray( config.value("graph").toObject()
                                                        .value("color1").toArray() );
    m_graph.colorEndGradient = graph::colorFromArray( config.value("graph").toObject()
                                                      .value("color2").toArray() );
    m_graph.watermarkFont.font_face = m_comm.font_face;
    m_graph.watermarkFont.color = graph::colorFromArray( config.value("graph").toObject()
                                                         .value("watermark").toObject()
                                                         .value("color").toArray() );
    m_graph.watermarkFont.size = config.value("graph").toObject().value("watermark").toObject()
            .value("font_size").toDouble();
    m_graph.watermarkFont.weight = config.value("graph").toObject().value("watermark").toObject()
            .value("font_weight").toDouble();

    // monitor box ----------------
    m_box.left = config.value("box").toObject().value("left").toDouble();
    m_box.right = config.value("box").toObject().value("right").toDouble();
    m_box.mid = config.value("box").toObject().value("mid").toDouble();
    m_box.border_width = config.value("box").toObject().value("border_width").toDouble();
    m_box.height = config.value("box").toObject().value("height").toDouble(80)
            - 3 * m_box.border_width;
    m_box.top_radius = config.value("gauge").toObject().value("padding").toDouble() +
            config.value("gauge").toObject().value("ext_radius").toDouble() +
            config.value("gauge").toObject().value("ext_border_width").toDouble() +
            config.value("box").toObject().value("border_width").toDouble();
    m_box.bot_radius = config.value("box").toObject().value("bot_radius").toDouble();
    m_box.graph_width = 2 * config.value("gauge").toObject().value("ext_radius").toDouble() +
            config.value("box").toObject().value("bullet_cx").toDouble() +
            2 * config.value("bullet").toObject().value("ext_radius").toDouble() +
            3 * config.value("gauge").toObject().value("padding").toDouble();
    m_box.graph_height = m_box.height -
            3.1 * config.value("gauge").toObject().value("padding").toDouble() -
            config.value("bullet").toObject().value("ext_radius").toDouble() -
            config.value("box").toObject().value("bullet_cy").toDouble() * 2;
    m_box.color = graph::colorFromArray( config.value("box").toObject().value("color").toArray() );
    m_box.border_color = graph::colorFromArray( config.value("box").toObject()
                                                .value("border_color").toArray() );

    m_box.bullet_cx = config.value("box").toObject().value("bullet_cx").toDouble();
    m_box.bullet_cy = config.value("box").toObject().value("bullet_cy").toDouble();
    m_box.caption_cx = config.value("box").toObject().value("caption").toObject().value("cx").toDouble();
    m_box.value_cx = config.value("box").toObject().value("value").toObject().value("cx").toDouble();
    m_box.captionFont.font_face = m_comm.font_face;
    m_box.captionFont.color = graph::colorFromArray( config.value("box").toObject()
                                                     .value("caption").toObject().value("color").toArray() );
    m_box.captionFont.size = config.value("box").toObject().value("caption").toObject().value("font_size").toDouble();
    m_box.captionFont.weight = config.value("box").toObject().value("caption").toObject().value("font_weight").toDouble();

    m_box.valueFont.font_face = m_comm.font_face;
    m_box.valueFont.color = graph::colorFromArray( config.value("box").toObject()
                                                   .value("value").toObject().value("color").toArray() );
    m_box.valueFont.size = config.value("box").toObject().value("value").toObject().value("font_size").toDouble();
    m_box.valueFont.weight = config.value("box").toObject().value("value").toObject().value("font_weight").toDouble();
    m_box.top = 0.0;

    // gauge ---------------------
    m_gauge.span_angle = 180.0 - config.value("gauge").toObject().value("start_angle").toDouble();
    m_gauge.start_angle = 90.0 + config.value("gauge").toObject().value("start_angle").toDouble();
    m_gauge.thick_angle = config.value("gauge").toObject().value("thick_angle").toDouble();
    m_gauge.mid_angle = config.value("gauge").toObject().value("mid_angle").toDouble();

    m_gauge.ext_radius = config.value("gauge").toObject().value("ext_radius").toDouble();
    m_gauge.ext_border_color = graph::colorFromArray( config.value("gauge").toObject()
                                                      .value("ext_border_color").toArray() );
    m_gauge.ext_border_width = config.value("gauge").toObject().value("ext_border_width").toDouble();
    m_gauge.radius = config.value("gauge").toObject().value("radius").toDouble();
    m_gauge.thickness = config.value("gauge").toObject().value("thickness").toDouble();
    m_gauge.padding = config.value("gauge").toObject().value("padding").toDouble();
    m_gauge.color = graph::colorFromArray( config.value("gauge").toObject().value("color").toArray() );
    m_gauge.bg_color = graph::colorFromArray( config.value("gauge").toObject().value("bg_color").toArray() );

    m_gauge.titleFont.font_face = m_comm.font_face;
    m_gauge.titleFont.color = graph::colorFromArray( config.value("gauge").toObject()
                                                     .value("title").toObject().value("color").toArray() );
    m_gauge.titleFont.size = config.value("gauge").toObject().value("title").toObject()
            .value("font_size").toDouble();
    m_gauge.titleFont.weight = config.value("gauge").toObject().value("title").toObject()
            .value("font_weight").toDouble();

    m_gauge.title_bg_color = graph::colorFromArray( config.value("gauge").toObject()
                                                    .value("title").toObject().value("bg_color").toArray() );
    m_gauge.title_border_color = graph::colorFromArray( config.value("gauge").toObject()
                                                        .value("title").toObject().value("border_color").toArray() );
    m_gauge.title_border_width = config.value("gauge").toObject().value("title").toObject()
            .value("border_width").toDouble();
    m_gauge.title_radius = config.value("gauge").toObject().value("title").toObject()
            .value("radius").toDouble();

    // bullet --------------------
    m_bullet.color = graph::colorFromArray( config.value("bullet").toObject().value("color").toArray() );
    m_bullet.ext_border_width = config.value("bullet").toObject().value("ext_border_width").toDouble();
    m_bullet.ext_radius = config.value("bullet").toObject().value("ext_radius").toDouble();
    m_bullet.radius = config.value("bullet").toObject().value("radius").toDouble();

    // map bg --------------------
    m_bg.fileName = config.value("bg").toObject().value("img").toString("");

    if(m_bg.fileName.isEmpty())
        m_bg.fileName =  ":/images/world_map.png";

    m_bg.ok = m_bg.img.load( m_bg.fileName );

    if(m_bg.ok) {
        qreal w = m_bg.img.width();
        qreal h = m_bg.img.height();
        qreal coef = w/h;
        m_bg.rect.setLeft( config.value("bg").toObject().value("x").toDouble(0) );
        m_bg.rect.setTop( config.value("bg").toObject().value("y").toDouble(0) );
        m_bg.rect.setWidth( config.value("bg").toObject().value("width").toDouble(m_screenGeometry.width()) );
        m_bg.rect.setHeight( m_bg.rect.width() / coef );
    }

    // map --------------------------
    m_map.map_width = m_bg.rect.width() - 0.0240 * m_bg.rect.width();
    m_map.map_cx = m_bg.rect.left() - 0.0169 * m_bg.rect.width();
    m_map.map_cy = m_bg.rect.top() - 0.0300 * m_bg.rect.width();

    m_map.infoFont.font_face = m_comm.font_face;
    m_map.infoFont.color = graph::colorFromArray( config.value("info").toObject().value("color").toArray() );
    m_map.infoFont.size = config.value("info").toObject().value("font_size").toDouble();
    m_map.infoFont.weight = config.value("info").toObject().value("font_weight").toDouble();

    m_map.line_color = graph::colorFromArray( config.value("line").toObject().value("color").toArray() );
    m_map.line_width = config.value("line").toObject().value("width").toDouble();

    m_map.cityFont.font_face = m_comm.font_face;
    m_map.cityFont.color = graph::colorFromArray( config.value("city").toObject().value("color").toArray() );
    m_map.cityFont.size = config.value("city").toObject().value("font_size").toDouble();
    m_map.cityFont.weight = config.value("city").toObject().value("font_weight").toDouble();

    m_map.currentFont.font_face = m_comm.font_face;
    m_map.currentFont.color = graph::colorFromArray( config.value("current").toObject().value("color").toArray() );
    m_map.currentFont.size = config.value("current").toObject().value("font_size").toDouble();
    m_map.currentFont.weight = config.value("current").toObject().value("font_weight").toDouble();

    m_map.city_dot_color = graph::colorFromArray( config.value("city").toObject().value("dot_color").toArray() );
    m_map.city_dot_radius = config.value("city").toObject().value("dot_radius").toDouble();
    m_map.city_border_color = graph::colorFromArray( config.value("city").toObject().value("border_color").toArray() );
    m_map.city_dot_border = config.value("city").toObject().value("dot_border").toDouble();
    m_map.hide_caption_dot_color = graph::colorFromArray( config.value("city").toObject().value("hide_caption_dot_color").toArray() );
    m_map.hide_caption_dot_radius = config.value("city").toObject().value("hide_caption_dot_radius").toDouble();
    m_map.current_dot_color = graph::colorFromArray( config.value("current").toObject().value("dot_color").toArray() );
    m_map.current_alt_color = graph::colorFromArray( config.value("current").toObject().value("alt_color").toArray() );
    m_map.current_dot_radius = config.value("current").toObject().value("dot_radius").toDouble();
    m_map.current_dot_border = config.value("current").toObject().value("dot_border").toDouble();
    m_map.current_border_color = graph::colorFromArray( config.value("current").toObject().value("border_color").toArray() );

    // utc line
    m_utcLine.line_color = graph::colorFromArray( config.value("time_line").toObject().value("color").toArray() );
    m_utcLine.line_width = config.value("time_line").toObject().value("width").toDouble();
}
//==============================================================================
void Settings::updateMonitorConfig(const QJsonObject &config)
{
    emit toLog(tr("Load monitor config from JSON"), Log::LogInfo);

    m_monVec.clear();

    int mon_cnt = (config.value("monitor").toArray().size() < 1)
            ? 1 : config.value("monitor").toArray().size();

    m_box.width = (m_screenGeometry.width() - m_box.left - m_box.right -
                   (mon_cnt - 1) * m_box.mid ) / mon_cnt;
    m_box.graph_width = m_box.width - m_box.graph_width;

    int graph_count = static_cast<int>(m_box.graph_width - 5.0);
    QVector<qreal> graphVec;
    for(auto i=0; i<graph_count; ++i) {
        graphVec.append( 0.0 );
    }

    for(auto i=0; i<mon_cnt; ++i) {

        QJsonObject monObj = config.value("monitor").toArray().at(i).toObject();
        MonitorStruct monStruct;

        monStruct.x = m_box.left + i * ( m_box.width + m_box.mid );
        monStruct.y = m_box.top + m_box.border_width;
        monStruct.gauge_x = monStruct.x + m_box.top_radius;
        monStruct.gauge_y = monStruct.y + m_box.top_radius;
        monStruct.title_y = monStruct.y + m_gauge.padding +
                m_gauge.ext_border_width + m_box.border_width + m_gauge.title_radius +
                2 * m_gauge.title_border_width;

        monStruct.bul_x = monStruct.gauge_x + m_gauge.ext_radius +
                m_box.bullet_cx + m_bullet.ext_radius;

        monStruct.caption_x = monStruct.bul_x + m_bullet.ext_radius +
                m_box.caption_cx;
        monStruct.value_x = monStruct.x + m_box.width +
                m_box.value_cx;

        monStruct.title = monObj.value("title").toString();
        monStruct.name = monObj.value("name").toString();
        monStruct.arg = monObj.value("arg").toString();
        monStruct.max = monObj.value("max").toDouble(100);

        if(monStruct.name == "cmd") {

            for(auto j=0; j<monObj.value("sub").toArray().size(); ++j) {
                SubStruct ss;
                ss.title = monObj.value("sub").toArray().at(j).toObject().value("title").toString();
                ss.arg = monObj.value("sub").toArray().at(j).toObject().value("arg").toString();
                ss.value = "";
                monStruct.sub.append(ss);
                if (j == 3) break;
            }
        }
        else if(monStruct.name == "cpu") {

            int cpuCount = qBound(1, QThread::idealThreadCount(), 4);
            for(auto j=0; j<cpuCount; ++j) {
                SubStruct ss;
                ss.title = QString("Core %1:").arg(j+1);
                ss.arg = QString::number(j);
                ss.value = "0.00 %";
                monStruct.sub.append(ss);
            }
        }
        else if(monStruct.name == "mem") {

            SubStruct ss;
            ss.title = QString("RAM:");
            ss.value = QString("0.00 MB");
            monStruct.sub.append(ss);
            ss.title = QString("Free:");
            monStruct.sub.append(ss);
            ss.title = QString("Swap:");
            monStruct.sub.append(ss);
            ss.title = QString("Free:");
            monStruct.sub.append(ss);
        }
        else if(monStruct.name == "temp") {

            if(monStruct.arg.indexOf("hwmon") == 0) {
                monStruct.arg = QString("cat /sys/class/hwmon/" + monStruct.arg + "_input");
            }

            for(auto j=0; j<monObj.value("sub").toArray().size(); ++j) {
                if (j>3) break;
                QJsonObject obj = monObj.value("sub").toArray().at(j).toObject();
                SubStruct ss;
                ss.value = QString("0.00 °C");
                ss.title = obj.value("title").toString();
                ss.arg = obj.value("arg").toString();
                if(ss.arg.indexOf("hwmon") == 0) {
                    ss.arg = QString("cat /sys/class/hwmon/" + ss.arg + "_input");
                }
                monStruct.sub.append(ss);
            }
        }
        else if(monStruct.name == "fs") {

#if defined(Q_OS_WIN32)
            monStruct.dev = monStruct.arg;
#else
            QString outStr = "";
            if(system_utils::osCmd("df --output=target,source | grep /dev/ | grep '" +
                                   monStruct.arg + " ' | awk '{print $2}'", outStr, 1000)) {

                if(outStr.indexOf("/dev/mapper/") == 0) {
                    system_utils::osCmd( "readlink -e " + outStr, outStr, 1000 );
                }
                if(outStr.indexOf("/dev/") == 0) outStr.remove(0, 5);
                monStruct.dev = outStr.trimmed();

                emit toLog(tr("Mount point: '%1', Device name: '%2' ('%3')")
                           .arg(monStruct.arg).arg(monStruct.dev).arg(outStr), Log::LogDbg);
            }
#endif
            SubStruct ss;
            ss.title = QString("Size:");
            ss.value = QString("0.00 MB");
            monStruct.sub.append(ss);
            ss.title = QString("Free:");
            monStruct.sub.append(ss);
        }
        else if((monStruct.name == "up") || (monStruct.name == "down")) {

            monStruct.dev = monStruct.arg;
            SubStruct ss;
            ss.title = QString("Speed:");
            ss.value = QString("0.00 MB");
            monStruct.sub.append(ss);
            ss.title = QString("Count:");
            monStruct.sub.append(ss);
        }

        for(auto j=0; j<monStruct.sub.size(); ++j) {

            monStruct.sub[j].y = 2.0 + m_box.mid +
                    m_bullet.ext_radius + m_box.bullet_cy * j;
        }

        if(monStruct.sub.size() < 3) {

            monStruct.graph.append( graphVec );
            monStruct.graph_x = monStruct.gauge_x + m_gauge.ext_radius +
                    m_box.bullet_cx + m_bullet.ext_radius;
            monStruct.graph_y = monStruct.y + m_box.mid +
                    m_bullet.ext_radius / 1.2 + m_box.bullet_cy * 2;
        }

        m_monVec.append(monStruct);
    }
}
//==============================================================================
void Settings::updateWorldmapConfig(const QJsonObject &config)
{
    emit toLog(tr("Load worldmap config from JSON"), Log::LogInfo);

    constexpr int default_offset = 24 * 60;
    m_miniCityVec.clear();
    m_cityVec.clear();
    QJsonArray city_coord = config.value("city_coord").toArray();

    for(auto i=0; i<city_coord.size(); ++i) {
        QJsonObject obj = city_coord.at(i).toObject();
        if(obj.value("city").isNull()
                || obj.value("city").toString("").isEmpty()) continue;

        CityStruct city;
        city.city = obj.value("city").toString();
        city.lat = obj.value("lat").toDouble();
        city.lon = obj.value("lon").toDouble();
        city.minutes_offset = obj.value("minute_offset").toInt(default_offset);
        city.point = geo::coordGeoToMap( city.lat, city.lon,
                                         m_map.map_width, m_map.map_cx, m_map.map_cy );

        if( !obj.contains("min_width") ||
                (m_bg.rect.width() > obj.value("min_width").toDouble()) ) {
            m_cityVec.append(city);
        }
        else {
            m_miniCityVec.append(city);
        }
    }
}
//==============================================================================
void Settings::reloadSettings()
{
    emit startReading();
    readSettings();
    emit finishReading();
}
//==============================================================================
void Settings::on_fileChanged(const QString &fileName)
{
    Q_UNUSED(fileName)
    system_utils::pause(1000);
    reloadSettings();
}
//==============================================================================
