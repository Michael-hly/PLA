#include "main.h"


typedef struct Point
{
	double x;
	double y;
}Point;

typedef struct Points
{
	int t;
	double y;
	Points() {}

	Points(int t, double y) :
		t(t), y(y) {}
}Points;


typedef struct Line
{
	double slope;
	double offset;
}Line;

Point pointCreate(double xVal, double yVal);
void pointCreate(Point& newPoint, double xVal, double yVal);

Line lineCreate(double slopeVal, double offsetVal);

void lineCreate(Line& newLine, const Point& point1, const Point& point2);

