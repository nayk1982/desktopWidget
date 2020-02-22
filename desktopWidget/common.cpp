#include "nayk/Convert"
#include "common.h"
//==============================================================================
QString getValueWithEd(qreal value)
{
    constexpr qreal mb = 1024.0;
    constexpr qreal gb = mb * 1024.0;
    constexpr qreal tb = gb * 1024.0;

    if(value > tb) return convert::doubleToStr( value / tb, 2 ) + " TB";
    else if(value > gb) return convert::doubleToStr( value / gb, 2 ) + " GB";
    else if(value > mb) return convert::doubleToStr( value / mb, 2 ) + " MB";
    else return convert::doubleToStr( value, 2 ) + " KB";
}
//==============================================================================
bool isEqualPoints(const QPointF &point1, const QPointF &point2)
{
    constexpr qreal pointEpsilon = 3.0;

    return (qAbs( point1.x() - point2.x() ) < pointEpsilon)
           &&
           (qAbs( point1.y() - point2.y() ) < pointEpsilon);
}
//==============================================================================
