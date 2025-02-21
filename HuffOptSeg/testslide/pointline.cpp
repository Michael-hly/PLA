#include "pointline.h"
#include "segmentset.h"

Point pointCreate(double xVal, double yVal)
{
	Point newPoint;
	newPoint.x = xVal;
	newPoint.y = yVal;
	return newPoint;
}



void pointCreate(Point &newPoint, double xVal, double yVal)
{
	newPoint.x=xVal;
	newPoint.y=yVal;
	return;
}

Line lineCreate(double xVal1, double yVal1, double xVal2, double yVal2)
{
	Line newLine;
	if(xVal1==xVal2)
	{
		newLine.slope = 0;
		newLine.offset = (yVal1+yVal2)/2;
	}
	else
	{
		newLine.slope = (yVal1 - yVal2)/(xVal1 - xVal2);
		newLine.offset= (xVal1*yVal2 - yVal1*xVal2)/(xVal1 - xVal2);
	}
	return newLine;
}

Line lineCreate(const Point &point1, const Point &point2)
{
	Line newLine;
	if(point1.x==point2.x)
	{
		newLine.slope=0;
		newLine.offset=(point1.y+point2.y)/2;
	}
	else
	{
		newLine.slope = (point1.y - point2.y)/(point1.x - point2.x);
		newLine.offset= (point1.x*point2.y - point1.y*point2.x)/(point1.x - point2.x);
	}
	return newLine;
}

Line lineCreate(double slopeVal, const Point &point)
{
	Line newLine;
	newLine.slope = slopeVal;
	newLine.offset = point.y - slopeVal * point.x;
	return newLine;
}

Line lineCreate(double slopeVal, double offsetVal)
{
	Line newLine;
	newLine.slope = slopeVal;
	newLine.offset = offsetVal;
	return newLine;
}

void lineCreate(Line &newLine, double xVal1, double yVal1, double xVal2, double yVal2)
{
	if(xVal1==xVal2)
	{
		newLine.slope = 0;
		newLine.offset = (yVal1+yVal2)/2;
	}
	else
	{
		newLine.slope = (yVal1 - yVal2)/(xVal1 - xVal2);
		newLine.offset= (xVal1*yVal2 - yVal1*xVal2)/(xVal1 - xVal2);
	}
	return;
}

void lineCreate(Line &newLine, const Point &point1, const Point &point2)
{
	if(point1.x==point2.x)
	{
		newLine.slope=0;
		newLine.offset=(point1.y+point2.y)/2;
	}
	else
	{
		newLine.slope = (point1.y - point2.y)/(point1.x - point2.x);
		newLine.offset= (point1.x*point2.y - point1.y*point2.x)/(point1.x - point2.x);
	}
	return;
}

void lineCreate(Line &newLine, double slopeVal, double offsetVal)
{
	newLine.slope = slopeVal;
	newLine.offset = offsetVal;
	return;
}

Point lineIntersection(const Line &line1, const Line &line2)
{
	Point interP;
	if(line1.slope==line2.slope)
	{
		interP.x=0;
		interP.y=line1.offset;
	}
	else
	{
		interP.x = (line1.offset - line2.offset)/(line2.slope - line1.slope);
		interP.y = (line2.slope*line1.offset - line1.slope*line2.offset)/(line2.slope - line1.slope);
	}
	return interP;
}

void lineIntersection(Point &interP, const Line &line1, const Line &line2)
{
	if(line1.slope==line2.slope)
	{
		interP.x=0;
		interP.y=line1.offset;
	}
	else
	{
		interP.x = (line1.offset - line2.offset)/(line2.slope - line1.slope);
		interP.y = (line2.slope*line1.offset - line1.slope*line2.offset)/(line2.slope - line1.slope);
	}
	return;
}

double linePoint(const Line &line, double xVal)
{
	double yVal;
	yVal = line.slope*xVal+line.offset;
	return yVal;
}

double getSlope(const Point &point1, const Point &point2)
{
	double slope;
	if(point1.x == point2.x)
	{
		slope = 0;
	}
	else
	{
		slope=(point1.y - point2.y)/(point1.x - point2.x);
	}
	return slope;
}



