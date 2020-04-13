#include <QtMath>
#include <QDesktopServices>
#include <QPainter>
#include <QMouseEvent>
//
#include "nayk/AppCore"
#include "nayk/FileSys"
#include "nayk/Convert"
#include "nayk/Geo"
#include "nayk/Graph"
#include "nayk/SystemUtils"
#include "nayk/GuiUtils"
//
#include "canvas.h"
#include "common.h"
#include "monitorobject.h"
#include "worldmapobject.h"
#include "currentlocationobject.h"
#include "utclineobject.h"
#include "utctextobject.h"
//==============================================================================

using namespace nayk;
using namespace file_sys;

//==============================================================================
Canvas::Canvas(QWidget *parent)
    : QGraphicsView(parent)
{
    setAttribute( Qt::WA_QuitOnClose );
    setAttribute( Qt::WA_TranslucentBackground );
    setWindowFlags( Qt::Tool | Qt::WindowStaysOnBottomHint | Qt::FramelessWindowHint );
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    m_log = new Log(this);
    connect(this, &Canvas::toLog, m_log, &Log::saveToLog, Qt::QueuedConnection);

    m_settings = new Settings(this);
    connect(m_settings, &Settings::toLog, m_log, &Log::saveToLog, Qt::QueuedConnection);
    connect(m_settings, &Settings::startReading, this, &Canvas::on_settingsStartReading);
    connect(m_settings, &Settings::finishReading,
            this, &Canvas::on_settingsFinishReading, Qt::QueuedConnection);

    m_scene = new CustomScene(this);
    setScene(m_scene);
    connect(m_scene, &CustomScene::toLog, m_log, &Log::saveToLog, Qt::QueuedConnection);
    connect(m_scene, &CustomScene::rightButtonClick, this, &Canvas::on_sceneRightMouseClick);

    m_timer.setInterval(15000);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &Canvas::on_timerTimeOut);

    setupMenu();
    installEventFilter(this);
    m_settings->reloadSettings();
}
//==============================================================================
Canvas::~Canvas()
{
    emit toLog(tr("Destructor ~Canvas()"), Log::LogDbg);

    if(m_scene) {
        m_scene->clear();
        delete m_scene;
    }

    if(m_settings) delete m_settings;
    if(m_log) delete m_log;
}
//==============================================================================
void Canvas::setupMenu()
{
    actionOpenMap = new QAction(tr("Open map"), this);
    connect(actionOpenMap, &QAction::triggered, this, &Canvas::on_actionOpenMapTriggered);
    m_menu.addAction(actionOpenMap);
    m_menu.addSeparator();
    m_menu.addAction( tr("Reload settings"), m_settings, &Settings::reloadSettings );
    m_menu.addAction( tr("About"), this, &Canvas::on_actionAboutTriggered );
    m_menu.addSeparator();
    m_menu.addAction( tr("Exit"), this, &Canvas::quit );
}
//==============================================================================
void Canvas::calcUtcLine(QDateTime curTime, QDateTime zeroTime,
                         UtcLineStruct &utcLine, bool saveLog)
{
    constexpr qreal minuteDeg = 360.0 / (24.0 * 60.0);
    utcLine.minute_offset = static_cast<qreal>(curTime.secsTo( zeroTime )) / 60.0;
    utcLine.lon = utcLine.minute_offset * minuteDeg;

    QPointF point = geo::coordGeoToMap( 0.0, utcLine.lon,
            m_settings->map().map_width,
            m_settings->map().map_cx, m_settings->map().map_cy );
    utcLine.x = point.x();
    utcLine.date = zeroTime.date();

    if(saveLog) {
        emit toLog(tr("date_time=%1, zero_date=%2, minutes_offset=%3, "
                      "longitude=%4, map_x=%5")
                   .arg(curTime.toString("yyyy-MM-dd_HH:mm:ss"))
                   .arg(utcLine.date.toString("yyyy-MM-dd"))
                   .arg(utcLine.minute_offset)
                   .arg(utcLine.lon)
                   .arg(utcLine.x), Log::LogDbg);
    }
}
//==============================================================================
void Canvas::updateUtcLongitude(bool saveLog)
{
    if(saveLog)
        emit toLog(tr("Update current 00:00 UTC position"), Log::LogInfo);

    QDateTime utcDateTime = QDateTime::currentDateTimeUtc();
    QDateTime zeroUtc = utcDateTime;
    zeroUtc.setTime( QTime(0, 0) );

    calcUtcLine(utcDateTime, zeroUtc, m_utcLine[0]);
    calcUtcLine(utcDateTime, zeroUtc.addDays(1), m_utcLine[1]);
    emit utcDataChanged0(m_utcLine[0].date, m_utcLine[0].lon);
    emit utcDataChanged1(m_utcLine[1].date, m_utcLine[1].lon);

    qreal y = m_settings->screenGeometry().height() - m_settings->box().height - 40.0;
    for(auto item:  m_scene->items()) {

        QGraphicsObject *obj = static_cast<QGraphicsObject *>(item);
        if(!obj) continue;

        if(obj->objectName() == "utcLineObject0") {
            obj->setPos( m_utcLine[0].x, 20.0 );
        }
        else if(obj->objectName() == "utcLineObject1") {
            obj->setPos( m_utcLine[1].x, 20.0 );
        }
        else if(obj->objectName() == "utcTextObject0") {
            obj->setPos( m_utcLine[0].x + 10.0, y );
        }
        else if(obj->objectName() == "utcTextObject1") {
            obj->setPos( m_utcLine[1].x + 10.0, y );
        }

        if(saveLog)
            emit toLog(tr("Find object %1").arg(obj->objectName()), Log::LogDbg);
    }
}
//==============================================================================
void Canvas::gotoYandexMap(QPointF p)
{
    QString url = "https://maps.yandex.ru/?ll=" + convert::doubleToStr(p.x(), 6)
            + "%2C" + convert::doubleToStr(p.y(), 6) + "&z=7&l=map";
    QDesktopServices::openUrl( QUrl(url) );
    emit toLog( url, Log::LogOut );
}
//==============================================================================
bool Canvas::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneWheel) {
        //QWheelEvent *wEvent = static_cast<QWheelEvent *>(event);
        emit toLog(tr("GraphicsSceneWheel event"), Log::LogDbg);
        return true;
    }
    else if (event->type() == QEvent::Scroll) {
        emit toLog(tr("Scroll event"), Log::LogDbg);
        return true;
    }
    else if (event->type() == QEvent::Wheel) {
        emit toLog(tr("Wheel event"), Log::LogDbg);
        return true;
    }
    else if(event->type() == QEvent::KeyPress) {
        //QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        emit toLog(tr("KeyPress event"), Log::LogDbg);
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
        emit toLog(tr("MouseButtonRelease event"), Log::LogDbg);
        m_mouseCoord = mEvent->pos();

        if(mEvent->button() == Qt::RightButton) {

            if(m_settings->mapVisible()) {
                m_geoCoord = geo::coordMapToGeo( m_mouseCoord.x(), m_mouseCoord.y(),
                                                 m_settings->map().map_width,
                                                 m_settings->map().map_cx,
                                                 m_settings->map().map_cy );
                actionOpenMap->setText(tr("Open map (lat: %1, lon: %2)")
                                       .arg(convert::doubleToStr(m_geoCoord.y(),6))
                                       .arg(convert::doubleToStr(m_geoCoord.x(),6))
                                       );
            }
            m_menu.popup(m_mouseCoord);
        }
        return true;
    }
    else if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
        emit toLog(tr("MouseButtonDblClick event"), Log::LogDbg);
        m_mouseCoord = mEvent->pos();

        if((mEvent->button() == Qt::LeftButton) && m_settings->mapVisible()) {
            m_geoCoord = geo::coordMapToGeo( m_mouseCoord.x(), m_mouseCoord.y(),
                                            m_settings->map().map_width,
                                            m_settings->map().map_cx,
                                            m_settings->map().map_cy );
            gotoYandexMap(m_geoCoord);
        }
        return true;
    }
    else {
        return QObject::eventFilter(obj, event);
    }
}
//==============================================================================
void Canvas::on_timerTimeOut()
{
    updateUtcLongitude();
}
//==============================================================================
void Canvas::on_settingsStartReading()
{
    emit toLog(tr("Canvas slot on_settingsStartReading()"), Log::LogDbg);
    m_timer.stop();
    m_scene->clear();
}
//==============================================================================
void Canvas::on_settingsFinishReading()
{
    emit toLog(tr("Canvas slot on_settingsFinishReading()"), Log::LogDbg);

    m_log->setDebugSave(m_settings->debug());
    m_log->deleteOldLogFiles();

    m_scene->clear();

    setFixedSize( m_settings->screenGeometry().width(), m_settings->screenGeometry().height() );
    setGeometry( m_settings->screenGeometry() );
    m_scene->setSceneRect( 0, 0, geometry().width(), geometry().height() );

    actionOpenMap->setVisible(m_settings->mapVisible());

    if(m_settings->mapVisible()) {

        WorldMapObject *worldMapObject = new WorldMapObject(m_settings);
        worldMapObject->setObjectName("worldMapObject");
        worldMapObject->setPos(0, 0);
        worldMapObject->setZValue(0);
        m_scene->addItem(worldMapObject);

        connect(worldMapObject, &WorldMapObject::toLog,
                m_log, &Log::saveToLog, Qt::QueuedConnection);

        for(int i=0; i<2; ++i) {

            m_utcLine[i] = m_settings->utcLine();
            UtcLineObject *utcLineObject = new UtcLineObject(m_settings);
            utcLineObject->setObjectName(QString("utcLineObject%1").arg(i));
            utcLineObject->setPos(0,0);
            utcLineObject->setZValue(1);
            m_scene->addItem(utcLineObject);

            connect(utcLineObject, &UtcLineObject::toLog,
                    m_log, &Log::saveToLog, Qt::QueuedConnection);

            UtcTextObject *utcTextObject = new UtcTextObject(m_settings);
            utcTextObject->setObjectName(QString("utcTextObject%1").arg(i));
            utcTextObject->setPos(0,0);
            utcTextObject->setZValue(2);
            m_scene->addItem(utcTextObject);

            connect(utcTextObject, &UtcTextObject::toLog,
                    m_log, &Log::saveToLog, Qt::QueuedConnection);

            if(i == 0)
                connect(this, &Canvas::utcDataChanged0, utcTextObject, &UtcTextObject::dataChanged);
            else
                connect(this, &Canvas::utcDataChanged1, utcTextObject, &UtcTextObject::dataChanged);
        }

        CurrentLocationObject *currentLocationObject = new CurrentLocationObject(m_settings);
        currentLocationObject->setObjectName("currentLocationObject");
        currentLocationObject->setPos(m_settings->currentLocation().point);
        currentLocationObject->setZValue(10);
        m_scene->addItem(currentLocationObject);

        connect(currentLocationObject, &CurrentLocationObject::toLog,
                m_log, &Log::saveToLog, Qt::QueuedConnection);
        connect(currentLocationObject, &CurrentLocationObject::locationChanged,
                this, &Canvas::on_currentLocationChanged);
        connect(currentLocationObject, &CurrentLocationObject::locationChanged,
                worldMapObject, &WorldMapObject::currentLocationChanged);

    }

    MonitorObject *monitorObject = new MonitorObject(m_settings);
    monitorObject->setObjectName("monitorObject");
    monitorObject->setPos( 0, static_cast<qreal>(geometry().height())
                 - m_settings->box().height - 3.0 * m_settings->box().border_width );
    monitorObject->setZValue(99);
    m_scene->addItem(monitorObject);

    connect(monitorObject, &MonitorObject::toLog,
            m_log, &Log::saveToLog, Qt::QueuedConnection);

    if(m_settings->mapVisible()) {
        updateUtcLongitude(true);
        m_timer.start();
    }
}
//==============================================================================
void Canvas::on_actionAboutTriggered()
{
    emit toLog(tr("Canvas slot on_actionAboutTriggered()"), Log::LogDbg);
    gui_utils::showAboutDialog(qApp->applicationName(),
                               tr("Evgeny Teterin"),
                               QString(),
                               false);
}
//==============================================================================
void Canvas::on_actionOpenMapTriggered()
{
    emit toLog(tr("Canvas slot on_actionOpenMapTriggered()"), Log::LogDbg);
    gotoYandexMap(m_geoCoord);
}
//==============================================================================
void Canvas::on_currentLocationChanged(const CurrentLocationStruct &currentLocation)
{
    emit toLog(tr("Canvas slot on_currentLocationChanged()"), Log::LogDbg);
    CurrentLocationObject *currentLocationObject
            = static_cast<CurrentLocationObject *>(sender());
    if(currentLocationObject) {

        qreal half = m_settings->map().current_dot_radius
                + m_settings->map().current_dot_border;
        currentLocationObject->setPos(currentLocation.point.x() - half,
                                      currentLocation.point.y() - half);
    }
}
//==============================================================================
void Canvas::on_sceneRightMouseClick(QPoint pos)
{
    m_mouseCoord = pos;
    if(m_settings->mapVisible()) {
        m_geoCoord = geo::coordMapToGeo( m_mouseCoord.x(), m_mouseCoord.y(),
                                         m_settings->map().map_width,
                                         m_settings->map().map_cx,
                                         m_settings->map().map_cy );
        actionOpenMap->setText(tr("Open map (lat: %1, lon: %2)")
                               .arg(convert::doubleToStr(m_geoCoord.y(),6))
                               .arg(convert::doubleToStr(m_geoCoord.x(),6))
                               );
    }
    m_menu.popup(m_mouseCoord);
}
//==============================================================================
