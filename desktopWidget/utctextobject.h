#ifndef UTCTEXTOBJECT_H
#define UTCTEXTOBJECT_H

#include <QGraphicsObject>
//
#include "nayk/Log"
//
#include "common.h"
#include "settings.h"
//==============================================================================
class UtcTextObject : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit UtcTextObject(Settings *settings);
    virtual ~UtcTextObject();

signals:
    void toLog(const QString &text, nayk::Log::LogType logType = nayk::Log::LogInfo);

public slots:
    void dataChanged(QDate date, qreal longitude);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    Settings *m_settings;
    QRectF m_boundingRect;
    QString m_text;

    void drawText(QPainter* painter, qreal x, qreal y, const QString &text, const FontStruct &font,
                          Qt::Alignment align = Qt::AlignLeft | Qt::AlignTop, bool withShadow = true);
};
//==============================================================================
#endif // UTCTEXTOBJECT_H
