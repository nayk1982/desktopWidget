#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "nayk/Log"
//==============================================================================
class CustomScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit CustomScene(QObject *parent = nullptr);
    virtual ~CustomScene();

signals:
    void toLog(const QString &text, nayk::Log::LogType logType = nayk::Log::LogInfo);
    void rightButtonClick(QPoint pos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};
//==============================================================================
#endif // CUSTOMSCENE_H
