#include "pointline.h"

void convexInitialization(deque<Point> &upper, deque<Point> &lower, Point point1, Point point2, Point tpoint1, Point tpoint2);
int feasibleRegionUpdate(deque<Point> &convex_upper, deque<Point> &convex_lower, Point point1, Point point2);
void HuffOptSeg(ifstream &streamFile,double sigma, string fileName,string fname);

