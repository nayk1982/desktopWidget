#ifndef CURRENTLOCATIONOBJECT_H
#define CURRENTLOCATIONOBJECT_H

#include <QGraphicsObject>
#include <QTimer>
//
#include "nayk/Log"
//
#include "common.h"
#include "settings.h"
//==============================================================================
class CurrentLocationObject : public QGraphicsObject
{
    Q_OBJECT

public:
    CurrentLocationObject(Settings *settings);
    virtual ~CurrentLocationObject();

signals:
    void toLog(const QString &text, nayk::Log::LogType logType = nayk::Log::LogInfo);
    void locationChanged(const CurrentLocationStruct &currentLocation);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    Settings *m_settings;
    bool m_altDotColor {false};
    int m_counter {-1};
    QRectF m_boundingRect;
    QTimer m_timer;
    CurrentLocationStruct m_curLoc;

    void updateCurrentLocation();
    void checkCurrentLocation();

private slots:
    void on_timerTimeOut();
};
//==============================================================================
#endif // CURRENTLOCATIONOBJECT_H
