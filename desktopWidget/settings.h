#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QRect>
#include <QFileSystemWatcher>
#include <QJsonObject>

#include "nayk/Log"
#include "common.h"
//==============================================================================
class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);
    virtual ~Settings();
    QString appDir() const;
    QString profileDir() const;
    int displayNum() const;
    bool debug() const;
    QRect screenGeometry() const;
    MapStruct map() const;
    CommonStruct common() const;
    BgStruct bg() const;
    CurrentLocationStruct currentLocation() const;
    ShadowStruct shadow() const;
    BoxStruct box() const;
    GaugeStruct gauge() const;
    BulletStruct bullet() const;
    GraphStruct graph() const;
    UtcLineStruct utcLine() const;
    QVector<CityStruct> miniCityVec() const;
    QVector<CityStruct> cityVec() const;
    QVector<MonitorStruct> monVec() const;
    bool mapVisible() const;

signals:
    void toLog(const QString &text, nayk::Log::LogType logType = nayk::Log::LogInfo);
    void startReading();
    void finishReading();

private:
    bool m_debug {false};
    bool m_mapVisible {false};
    int m_displayNum {0};
    QRect m_screenGeometry;
    QString m_appDir {""};
    QString m_profileDir {""};
    QFileSystemWatcher m_fsWatcher;
    MapStruct m_map;
    CommonStruct m_comm;
    BgStruct m_bg;
    CurrentLocationStruct m_curLoc;
    ShadowStruct m_shadow;
    BoxStruct m_box;
    GaugeStruct m_gauge;
    BulletStruct m_bullet;
    GraphStruct m_graph;
    UtcLineStruct m_utcLine;
    QVector<CityStruct> m_miniCityVec;
    QVector<CityStruct> m_cityVec;
    QVector<MonitorStruct> m_monVec;

    void readSettings();
    bool loadJson(const QString &fileName, const QString &resFileName, QJsonObject &jsonObject);
    void updateCommonConfig( const QJsonObject &config );
    void updateMonitorConfig( const QJsonObject &config );
    void updateWorldmapConfig( const QJsonObject &config );

public slots:
    void reloadSettings();

private slots:
    void on_fileChanged(const QString &fileName);

};
//==============================================================================
#endif // SETTINGS_H
