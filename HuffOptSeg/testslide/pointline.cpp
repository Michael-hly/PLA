#include "pointline.h"
#include "segmentset.h"

Point pointCreate(double xVal, double yVal)
{
	Point newPoint;
	newPoint.x = xVal;
	newPoint.y = yVal;
	return newPoint;
}


void pointCreate(Point& newPoint, double xVal, double yVal)
{
	newPoint.x = xVal;
	newPoint.y = yVal;
	return;
}



Line lineCreate(double slopeVal, double offsetVal)
{
	Line newLine;
	newLine.slope = slopeVal;
	newLine.offset = offsetVal;
	return newLine;
}



void lineCreate(Line& newLine, const Point& point1, const Point& point2)
{
	if (point1.x == point2.x)
	{
		newLine.slope = 0;
		newLine.offset = (point1.y + point2.y) / 2;
	}
	else
	{
		newLine.slope = (point1.y - point2.y) / (point1.x - point2.x);
		newLine.offset = (point1.x * point2.y - point1.y * point2.x) / (point1.x - point2.x);
	}
	return;
}




