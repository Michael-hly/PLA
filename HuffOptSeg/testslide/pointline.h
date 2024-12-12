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

	Points(int t,double y) :
		t(t), y(y) {}
}Points;

typedef struct stPoint
{
    double x;
    double y;
    int    isSaved; 

    stPoint()
    {
        x = 0;
        y = 0;
        isSaved = 0;
    }
}stPoint;



typedef struct Line
{
	double slope;
	double offset;
}Line;


typedef struct Triple
{
	double t;
	double ts;
	double te;
}Triple;

Triple tripleCreate(double tVal, double tsVal, double teVal);
void coutTriple(deque<Triple> deTri);

Point pointCreate(double xVal, double yVal);
void pointCreate(Point &newPoint, double xVal, double yVal);

Line lineCreate(double xVal1, double yVal1, double xVal2, double yVal2);
Line lineCreate(const Point &point1, const Point &point2);
Line lineCreate(double slopeVal, double offsetVal);
void lineCreate(Line &newLine, double xVal1, double yVal1, double xVal2, double yVal2);
void lineCreate(Line &newLine, const Point &point1, const Point &point2);
void lineCreate(Line &newLine, double slopeVal, double offsetVal);

Point lineIntersection(const Line &line1, const Line &line2);
void lineIntersection(Point &interP, const Line &line1, const Line &line2);

double linePoint(const Line &line, double xVal);
double getSlope(const Point &point1, const Point &point2);
Line lineCreate(double slopeVal, const Point &point);
