#ifndef CANVAS_H
#define CANVAS_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QTimer>
#include <QMenu>
#include <QEvent>
#include "nayk/Log"
#include "settings.h"
#include "customscene.h"
//==============================================================================
class Canvas : public QGraphicsView
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);
    virtual ~Canvas();

signals:
    void toLog(const QString &text, nayk::Log::LogType logType = nayk::Log::LogInfo);
    void quit();
    void utcDataChanged0(QDate date, qreal longitude);
    void utcDataChanged1(QDate date, qreal longitude);

private:
    nayk::Log *m_log {nullptr};
    Settings *m_settings {nullptr};
    CustomScene *m_scene {nullptr};
    QAction *actionOpenMap {nullptr};
    QMenu m_menu;
    QTimer m_timer;
    UtcLineStruct m_utcLine[2];
    QPoint m_mouseCoord;
    QPointF m_geoCoord;

    void setupMenu();
    void calcUtcLine(QDateTime curTime, QDateTime zeroTime, UtcLineStruct &utcLine, bool saveLog = false);
    void updateUtcLongitude(bool saveLog = false);
    void gotoYandexMap(QPointF p);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_timerTimeOut();
    void on_settingsStartReading();
    void on_settingsFinishReading();
    void on_actionAboutTriggered();
    void on_actionOpenMapTriggered();
    void on_currentLocationChanged(const CurrentLocationStruct &currentLocation);
    void on_sceneRightMouseClick(QPoint pos);
};
//==============================================================================
#endif // CANVAS_H
