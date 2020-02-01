#ifndef COMMON_H
#define COMMON_H

#include <QtCore>
#include <QObject>
#include <QColor>
#include <QVector>
#include <QString>
#include <QImage>

//==============================================================================
const QString file_Config       = "main.conf";
const QString file_Style        = "main.qss";
const QString file_CommonJson   = "common.json";
const QString file_MonitorJson  = "monitor.json";
const QString file_WorldmapJson = "worldmap.json";

//==============================================================================
typedef struct BgStruct
{
    bool ok {false};
    QString fileName {""};
    QRectF rect {QRectF(0,0,0,0)};
    QImage img;

} BgStruct;
//==============================================================================
typedef struct FontStruct
{
    qreal size {9.0};
    qreal weight {0.5};
    QString font_face {"Monospace"};
    QColor color {QColor(0, 0, 0, 255)};

} FontStruct;
//==============================================================================
typedef struct CurrentLocationStruct
{
    qreal lat {0};
    qreal lon {0};
    QPointF point;
    QString city {""};
    QString ip {""};
    QString country {""};

} CurrentLocationStruct;
//==============================================================================
typedef struct CityStruct
{
    qreal lat {0};
    qreal lon {0};
    int minutes_offset {0};
    QPointF point;
    QString city {""};

} CityStruct;
//==============================================================================
typedef struct CommonStruct
{
    int update_current_sec {3600};
    QString api_key {""};
    QString api_username {""};
    QString font_face {"Monospace"};

} CommonStruct;
//==============================================================================
typedef struct MapStruct
{
    qreal map_width {0};
    qreal map_cx {0};
    qreal map_cy {0};
    qreal city_dot_radius;
    qreal city_dot_border;
    qreal line_width;
    qreal hide_caption_dot_radius;
    qreal current_dot_radius;
    qreal current_dot_border;
    QColor line_color;
    QColor city_dot_color;
    QColor city_border_color;
    QColor hide_caption_dot_color;
    QColor current_dot_color;
    QColor current_alt_color;
    QColor current_border_color;
    FontStruct infoFont;
    FontStruct cityFont;
    FontStruct currentFont;

} MapStruct;
//==============================================================================
typedef struct BoxStruct
{
    qreal width {100};
    qreal height {72.0};
    qreal top {0};
    qreal left {0};
    qreal right {0};
    qreal mid {0};
    qreal border_width {0.9};
    qreal top_radius {30};
    qreal bot_radius {6.0};
    qreal graph_width {100};
    qreal graph_height {20};
    qreal bullet_cx {8};
    qreal bullet_cy {8};
    qreal caption_cx {20};
    qreal value_cx {20};
    QColor color {QColor(6, 10, 9, 110)};
    QColor border_color {QColor(64, 186, 162, 178)};
    FontStruct captionFont;
    FontStruct valueFont;

} BoxStruct;
//==============================================================================
typedef struct SubStruct
{
    qreal y {0};
    QString arg {""};
    QString title {""};
    QString value {""};

} SubStruct;
//==============================================================================
typedef struct MonitorStruct
{
    qreal x {0};
    qreal y {0};
    qreal gauge_x {0};
    qreal gauge_y {0};
    qreal title_y {0};
    qreal bul_x {0};
    qreal caption_x {0};
    qreal value_x {0};
    qreal max {100};
    qreal value {0};
    qreal graph_max {100};
    qreal graph_value {0};
    qreal graph_x {0};
    qreal graph_y {0};
    QString title {""};
    QString name {""};
    QString arg {""};
    QString dev {""};
    QVector<qreal> graph;
    QVector<SubStruct> sub;

} MonitorStruct;
//==============================================================================
typedef struct GaugeStruct
{
    qreal ext_radius {0};
    qreal ext_border_width;
    qreal radius;
    qreal thickness;
    qreal padding;
    qreal start_angle;
    qreal span_angle;
    qreal thick_angle;
    qreal mid_angle;
    qreal title_border_width;
    qreal title_radius;
    QColor color;
    QColor bg_color;
    QColor ext_border_color;
    QColor title_bg_color;
    QColor title_border_color;
    FontStruct titleFont;

} GaugeStruct;
//==============================================================================
typedef struct BulletStruct
{
    qreal ext_radius;
    qreal ext_border_width;
    qreal radius;
    QColor color;

} BulletStruct;

//==============================================================================
typedef struct ShadowStruct
{
    qreal cx;
    qreal cy;
    QColor color;

} ShadowStruct;
//==============================================================================
typedef struct GraphStruct
{
    qreal border_width;
    qreal border_radius;
    QColor bg_color;
    QColor border_color;
    QColor colorStartGradient;
    QColor colorEndGradient;
    FontStruct watermarkFont;

} GraphStruct;
//==============================================================================
typedef struct UtcLineStruct
{
    qreal lon {0};
    qreal minute_offset {0};
    qreal x {0};
    qreal line_width {0.5};
    QColor line_color;
    QDate date;

} UtcLineStruct;
//==============================================================================

QString getValueWithEd(qreal value);
bool isEqualPoints(const QPointF &point1, const QPointF &point2);

//==============================================================================
#endif // COMMON_H
