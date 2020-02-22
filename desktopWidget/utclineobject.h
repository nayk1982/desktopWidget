#ifndef UTCLINEOBJECT_H
#define UTCLINEOBJECT_H

#include <QGraphicsObject>
//
#include "nayk/Log"
//
#include "common.h"
#include "settings.h"
//==============================================================================
class UtcLineObject : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit UtcLineObject(Settings *settings);
    virtual ~UtcLineObject();

signals:
    void toLog(const QString &text, nayk::Log::LogType logType = nayk::Log::LogInfo);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    Settings *m_settings;
    QRectF m_boundingRect;
};
//==============================================================================
#endif // UTCLINEOBJECT_H
