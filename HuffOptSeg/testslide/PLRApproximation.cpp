#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "fs.h"
#include "main.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <process.h>
#include <iomanip>
#include "windows.h"
#include "psapi.h"
#include <string>
#pragma comment(lib,"psapi.Lib")
#include "segmentset.h"
#include <map>
#include <set>
#include <unordered_map>
#include "Util.hpp"
using namespace std;

long datacout;


int feasibleRegionUpdate(deque<Point> &convex_upper, deque<Point> &convex_lower, Point point1, Point point2)
{


	//in order to remove the effect of division operation, we use the ulternatives in if and else

	//update point 1

	if((point1.x-convex_lower.back().x)*(convex_lower.back().y-convex_upper.front().y)-(point1.y-convex_lower.back().y)*(convex_lower.back().x-convex_upper.front().x)>derror)
	{
		return 0;
	}

	else if((point1.x-convex_upper.back().x)*(convex_upper.back().y-convex_lower.front().y)<=(point1.y-convex_upper.back().y)*(convex_upper.back().x-convex_lower.front().x))
	{
		//do nothing
	}
	else
	{
		while(convex_lower.size()>1 && (convex_lower[1].y-convex_lower[0].y)*(point1.x-convex_lower[0].x)>=(point1.y-convex_lower[0].y)*(convex_lower[1].x-convex_lower[0].x))
		{
			convex_lower.pop_front();
		}

		while(convex_upper.size()>1 && (convex_upper[convex_upper.size()-1].y-convex_upper[convex_upper.size()-2].y)*(point1.x-convex_upper[convex_upper.size()-1].x)>=(point1.y-convex_upper[convex_upper.size()-1].y)*(convex_upper[convex_upper.size()-1].x-convex_upper[convex_upper.size()-2].x))
		{
			convex_upper.pop_back();
		}

		convex_upper.push_back(point1);
	}

	//update point 2

	if((point2.x-convex_upper.back().x)*(convex_upper.back().y-convex_lower.front().y)-(point2.y-convex_upper.back().y)*(convex_upper.back().x-convex_lower.front().x)<-derror)
	{
		return 0;
	}

	else if((point2.x-convex_lower.back().x)*(convex_lower.back().y-convex_upper.front().y)>=(point2.y-convex_lower.back().y)*(convex_lower.back().x-convex_upper.front().x))
	{
		//do nothing
	}
	else
	{
		while(convex_upper.size()>1 && (convex_upper[1].y-convex_upper[0].y)*(point2.x-convex_upper[0].x)<=(point2.y-convex_upper[0].y)*(convex_upper[1].x-convex_upper[0].x))
		{
			convex_upper.pop_front();
		}

		while(convex_lower.size()>1 && (convex_lower[convex_lower.size()-1].y-convex_lower[convex_lower.size()-2].y)*(point2.x-convex_lower[convex_lower.size()-1].x)<=(point2.y-convex_lower[convex_lower.size()-1].y)*(convex_lower[convex_lower.size()-1].x-convex_lower[convex_lower.size()-2].x))
		{
			convex_lower.pop_back();

		}

		convex_lower.push_back(point2);
	}

	return 1;
}



void convexInitialization(deque<Point> &upper, deque<Point> &lower, Point point1, Point point2, Point tpoint1, Point tpoint2)
{
	upper.push_back(point1);upper.push_back(tpoint1);
	lower.push_back(point2);lower.push_back(tpoint2);
	return;
}




void mergeYofSegments(unordered_map<long, double>& TtoY, vector<GenerateSeg> segments, unordered_map<double, int>& frequencyMap);
bool compareByT(const GenerateSeg& seg1, const GenerateSeg& seg2);
double handle_double(double x, double y);
void mergeSegments(vector<SPSegment>& segments);
std::unordered_map<double, std::vector < std::pair<int, double >> > directmap(vector<SPSegment>& segments);
vector<unsigned char> toByteArray(unordered_map<double, std::vector < std::pair<int, double >> > &segmentsPerB, int lastx, bool lastexist, double lasty, unordered_map<double, int> &frequencyMap);
vector<decompresSeg> readbyte(vector<unsigned char>& bytes, int& lastTimeStamp, int& blastexist, float& blasty);
bool decompresscompareByT(const decompresSeg& seg1, const decompresSeg& seg2);
bool compareByVmin(const GenerateSeg& seg1, const GenerateSeg& seg2);

void HuffOptSeg(ifstream& streamFile, double sigma, string fileName, string fname)
{
	//first read all the sequence into memory
	deque<Point> initialStream;
	Point initialPoint;
	initialPoint = pointCreate(0, 0);
	long initialIndex = 1;
	double initialData;
	double maxVal, minVal;
	LARGE_INTEGER nFreq;
	QueryPerformanceFrequency(&nFreq);
	LARGE_INTEGER t1;
	LARGE_INTEGER t2,t3;
	double dt,dt1 = 0;
	double maxError = 0;

	maxVal = -1000000000;
	minVal = 10000000000;

	FILE* fp;
	fp = fopen(fileName.c_str(), "r");
	if (fp == NULL)
	{
		exit(1);
	}

	while (!feof(fp))
	{
		try
		{
			int f = fscanf(fp, "%lf", &initialData);
			pointCreate(initialPoint, initialIndex, initialData);

			if (f == 0)
			{
				exit(1);
			}
			if (maxVal < initialData)
			{
				maxVal = initialData;
			}
			if (minVal > initialData)
			{
				minVal = initialData;
			}
			initialStream.push_back(initialPoint);
			initialIndex++;
		}
		catch (exception e)
		{
			exit(1);
		}
	}
	fclose(fp);



	datacout = initialStream.size();
	double range= (maxVal - minVal);
	double epsilon = sigma;
	sigma = sigma * range;

	deque<Point> convex_upper, convex_lower; //convex_upper is located on the above

	Point point1, point2, tpoint1, tpoint2;
	Line tempU, tempL; //record the segment slope with maximum value
	vector<GenerateSeg> generateSegContainer;
	vector<decompresSeg>decomprsegVector;
	double streamdata;
	long sIndex; //record the index of the point under processing
	long startIndex; //record the index of the first point of a segment
	long sIndexOffset; //record the relative index of the point in current segment
	long numberSeg; //record the number of segments

	bool lastexist = false;
	int lastx = 0;
	lastx = datacout;
	double lasty = 0;

	tempU = lineCreate(0, 0);
	streamdata = 0;
	sIndex = 0;
	startIndex = 0;
	sIndexOffset = 0;
	numberSeg = 0;

	sIndex = 0;
	int i = 0;
	QueryPerformanceCounter(&t1);

	while (sIndex < datacout)
	{
		sIndex++;
		sIndexOffset++;
		streamdata = initialStream[sIndex - 1].y;
		pointCreate(point1, sIndex, streamdata + sigma);//upper point
		pointCreate(point2, sIndex, streamdata - sigma);//lower point
		if (sIndexOffset == 1)
		{
			startIndex = sIndex;
			if (sIndex < datacout)
			{
				sIndex++;
				sIndexOffset++;
				streamdata = initialStream[sIndex - 1].y;
				pointCreate(tpoint1, sIndex, streamdata + sigma);//upper point
				pointCreate(tpoint2, sIndex, streamdata - sigma);//lower point
				convexInitialization(convex_upper, convex_lower, point1, point2, tpoint1, tpoint2);
				continue;
			}
			else
			{
				lasty = initialStream[datacout - 1].y;
				lastexist = 1;
				cout << "Note the last segment contains only one point!" << endl;
				break;
			}
		}
		if (!feasibleRegionUpdate(convex_upper, convex_lower, point1, point2))
		{
			lineCreate(tempL, convex_upper.front(), convex_lower.back());
			lineCreate(tempU, convex_upper.back(), convex_lower.front());

			numberSeg++;

			convex_upper.clear(); convex_lower.clear();

			generateSegContainer.resize(i + 1);
			generateSegContainer[i].timestart = startIndex;
			generateSegContainer[i].vstartup = tempL.slope * startIndex + tempL.offset;
			generateSegContainer[i].vstartlow = tempU.slope * startIndex + tempU.offset;

			i++;
			sIndexOffset = 1;
			startIndex = sIndex;
			if (sIndex < datacout)
			{
				sIndex++;
				sIndexOffset++;
				streamdata = initialStream[sIndex - 1].y;
				pointCreate(tpoint1, sIndex, streamdata + sigma);//upper point
				pointCreate(tpoint2, sIndex, streamdata - sigma);//lower point
				convexInitialization(convex_upper, convex_lower, point1, point2, tpoint1, tpoint2);
				continue;
			}
			else
			{
				//std::cout << "Note the last segment contains only one point!" << endl;
				lasty = initialStream[datacout - 1].y;
				lastexist = 1;

				break;
			}
		}
		else
		{

			lineCreate(tempL, convex_upper.front(), convex_lower.back());
			lineCreate(tempU, convex_upper.back(), convex_lower.front());

		}
	}

	if (sIndex > 0)
	{
		if (sIndexOffset == 1)
		{
			numberSeg++;
		}
		else
		{
			lineCreate(tempL, convex_upper.front(), convex_lower.back());
			lineCreate(tempU, convex_upper.back(), convex_lower.front());
			
			numberSeg++;

			generateSegContainer.resize(i + 1);
			generateSegContainer[i].timestart = startIndex;
			generateSegContainer[i].vstartup = tempL.slope * startIndex + tempL.offset;
			generateSegContainer[i].vstartlow = tempU.slope * startIndex + tempU.offset;

		}
	}
	QueryPerformanceCounter(&t2);
	

	unordered_map<long, double> TtoY;
	unordered_map<double, int> frequencyMap;
	mergeYofSegments(TtoY,generateSegContainer,frequencyMap);
	vector<SPSegment> segments;
	double aMax = 100000000;
	double aMin = -100000000;
	long segmentSize = 1;
	long segmentInitTimestamp =initialStream[0].x;
	double segmentInitValue = TtoY[generateSegContainer[0].timestart];
	int slidenum = 1;
	for (int i = 1; i < datacout; i++) {
		Point point = initialStream[i];
		segmentSize++;
		double upValue = point.y + sigma;
		double downValue = point.y - sigma;
		double aMaxTemp = (upValue - segmentInitValue) /(point.x - segmentInitTimestamp);
		double aMinTemp = (downValue - segmentInitValue) /(point.x - segmentInitTimestamp);
		if (segmentSize == 2) {
			aMax = aMaxTemp;
			aMin = aMinTemp;
		}
		else {
			double upLim = aMax * (point.x - segmentInitTimestamp) + segmentInitValue;
			double downLim = aMin * (point.x - segmentInitTimestamp) + segmentInitValue;
			if (point.x == generateSegContainer[slidenum].timestart) {
				segments.push_back(SPSegment(segmentInitTimestamp, aMin, aMax, segmentInitValue));
				segmentSize = 1;
				segmentInitTimestamp = generateSegContainer[slidenum].timestart;
				segmentInitValue = TtoY[generateSegContainer[slidenum].timestart];
				slidenum++;
				continue;
			}
			if (upValue < upLim)
				aMax = (((aMaxTemp) > (aMin)) ? (aMaxTemp) : (aMin));
			if (downValue > downLim)
				aMin = (((aMinTemp) < (aMax)) ? (aMinTemp) : (aMax));
		}
		if (lastexist == 1 && (i == datacout - 1)) {
			break;
		}
	}
	if (segmentSize != 0){
		segments.push_back(SPSegment(segmentInitTimestamp, aMin, aMax, segmentInitValue));
	}
	TtoY.clear();
	generateSegContainer.clear();

	mergeSegments(segments);
	std::unordered_map<double, std::vector < std::pair<int, double >> >resultmap= directmap(segments);
	vector<unsigned char>binary=toByteArray(resultmap, lastx, lastexist, lasty, frequencyMap);
	frequencyMap.clear();
	int lastTimeStamp = 0;
	float blasty = 0;
	int blastexist = 0;
	decomprsegVector = readbyte(binary, lastTimeStamp, blastexist, blasty);
	

	vector<Points> points;
	long long currentTimeStamp = 1;
	for (int i = 0; i < decomprsegVector.size() - 1; i++) {
		while (currentTimeStamp < decomprsegVector[i + 1].tstart) {
			points.push_back(Points(currentTimeStamp, decomprsegVector[i].avra * (currentTimeStamp - decomprsegVector[i].tstart) + decomprsegVector[i].avrv));
			currentTimeStamp++;
		}
	}
	while (currentTimeStamp < lastTimeStamp) {
		points.push_back(Points(currentTimeStamp, decomprsegVector[decomprsegVector.size()-1].avra * (currentTimeStamp - decomprsegVector[decomprsegVector.size() - 1].tstart) + decomprsegVector[decomprsegVector.size() - 1].avrv));
		currentTimeStamp++;
	}
	if (blastexist == 1) {
		points.push_back(Points(lastTimeStamp, blasty));
	}
	else
		points.push_back(Points(currentTimeStamp, decomprsegVector[decomprsegVector.size() - 1].avra * (currentTimeStamp - decomprsegVector[decomprsegVector.size() - 1].tstart) + decomprsegVector[decomprsegVector.size() - 1].avrv));
	
	QueryPerformanceCounter(&t3);
	
	
	double MAE = 0;
	double RMSE = 0;
	for (int i = 0; i < datacout; i++) {
		double dif = abs(points[i].y - initialStream[i].y);
		MAE += dif;
		RMSE += dif* dif;
		if (maxError < dif) {
			maxError = dif;
		}
	}
	MAE = MAE / datacout;
	RMSE = sqrt(RMSE / datacout);
	if (maxError >1.001 * sigma) {
		cout << "maxError - delta =" << maxError - sigma << endl;
		cout << "delta: " << sigma << endl;
	}
	else
		cout << "fit error bound" << endl;
	

	dt = 1000000 * (t2.QuadPart - t1.QuadPart) / (double)nFreq.QuadPart;
	dt1= 1000000 * (t3.QuadPart - t1.QuadPart) / (double)nFreq.QuadPart;

	cout <<"Epsilon: "<<epsilon*100 << "%, Compression ratio: " << (double)datacout * 8 / binary.size() << endl;
	//cout<<"MAE: "<< MAE <<", RMSE: "<<RMSE<< endl;
	//cout << "Time cost: " << dt1 / 1000 << "ms"<< endl;
	cout << endl;
	return;
}
bool compareByT(const GenerateSeg& seg1, const GenerateSeg& seg2) {
	return seg1.timestart < seg2.timestart;
}

bool compareByVmin(const GenerateSeg& seg1, const GenerateSeg& seg2) {
	return seg1.vstartlow < seg2.vstartlow;
}
void mergeYofSegments(unordered_map<long, double> &TtoY,vector<GenerateSeg> segments, unordered_map<double, int>& frequencyMap) {

	vector<GenerateSeg> segmentRecord;
	sort(segments.begin(), segments.end(), compareByVmin);
	GenerateSeg mergedSeg = segments[0];
	GenerateSeg currentSeg = segments[0];
	segmentRecord.push_back(segments[0]);
	int cnt = 1;
	while (cnt < segments.size()) {
		currentSeg = segments[cnt];
		if (currentSeg.vstartlow <= mergedSeg.vstartup && currentSeg.vstartup >= mergedSeg.vstartlow) {
			segmentRecord.push_back(segments[cnt]);
			mergedSeg.vstartlow = (((mergedSeg.vstartlow) >= (currentSeg.vstartlow)) ? (mergedSeg.vstartlow) : (currentSeg.vstartlow));
			mergedSeg.vstartup = (((mergedSeg.vstartup) <= (currentSeg.vstartup)) ? (mergedSeg.vstartup) : (currentSeg.vstartup));
		}
		else {
			double y= handle_double(mergedSeg.vstartlow, mergedSeg.vstartup);
			frequencyMap[y] = segmentRecord.size();
			for (const GenerateSeg& seg : segmentRecord)
				TtoY[seg.timestart] = y;
			segmentRecord.clear();
			mergedSeg = currentSeg;
			segmentRecord.push_back(mergedSeg);
		}
		cnt++;
	}
	if (!segmentRecord.empty()) {
		double y = handle_double(mergedSeg.vstartlow, mergedSeg.vstartup);
		frequencyMap[y] = segmentRecord.size();
		for (const GenerateSeg& seg : segmentRecord)
			TtoY[seg.timestart] = y;
		segmentRecord.clear();
	}

}

void mergeSegments(vector<SPSegment>& segments) {
	std::vector<SPSegment> segmentsMerged;
	std::vector<SPSegment> segmentRecord;
	sort(segments.begin(), segments.end(), [](SPSegment& seg1, SPSegment& seg2) {
		return seg1.aMin < seg2.aMin;
		});
	SPSegment mergedSeg = segments[0];
	SPSegment currentSeg = segments[0];
	segmentRecord.push_back(segments[0]);
	int cnt = 1;
	while (cnt < segments.size()) {
		currentSeg = segments[cnt];
		if (currentSeg.aMin <= mergedSeg.aMax && currentSeg.aMax >= mergedSeg.aMin) {
			segmentRecord.push_back(segments[cnt]);
			mergedSeg.setAMin((((mergedSeg.aMin) > (currentSeg.aMin)) ? (mergedSeg.aMin) : (currentSeg.aMin)));
			mergedSeg.setAMax((((mergedSeg.aMax) < (currentSeg.aMax)) ? (mergedSeg.aMax) : (currentSeg.aMax)));
		}
		else {
			double a = handle_double(mergedSeg.aMin, mergedSeg.aMax);
			for (const SPSegment& seg : segmentRecord)
				segmentsMerged.push_back(SPSegment(seg.initTimestamp, a, mergedSeg.aMax, seg.b));
			segmentRecord.clear();
			mergedSeg = currentSeg;
			segmentRecord.push_back(mergedSeg);
		}
		cnt++;
	}
	if (!segmentRecord.empty()) {
		double a = handle_double(mergedSeg.aMin, mergedSeg.aMax);
		for (const SPSegment& seg : segmentRecord)
			segmentsMerged.push_back(SPSegment(seg.initTimestamp, a, mergedSeg.aMax, seg.b));
		segmentRecord.clear();
	}
	sort(segmentsMerged.begin(), segmentsMerged.end(), [](SPSegment& seg1, SPSegment& seg2) {
		return seg1.initTimestamp < seg2.initTimestamp;
		});
	segments = segmentsMerged;
	
}

std::unordered_map<double, std::vector < std::pair<int, double >> > directmap(vector<SPSegment>& segments) {
	std::unordered_map<double, std::vector < std::pair<int, double >> > atotandymap;
	for (const auto& segment : segments) {
		int t = segment.initTimestamp;
		double a = segment.aMin;
		double v = segment.b;
		atotandymap[a].push_back(std::make_pair(t, v));
	}

	return atotandymap;
}


vector<unsigned char> toByteArray(std::unordered_map<double, std::vector < std::pair<int, double >> > &segmentsPerB, int lastx, bool lastexist, double lasty, unordered_map<double, int> &frequencyMap)
{
	stringstream outputStream;
	vector<double> blist;
	int numA = segmentsPerB.size();
	vector<unsigned char> bytes;
	try {
		VariablebyteEncoder::write(numA, outputStream);
		int tbnum = 0;
		for (auto& segmentPerB : segmentsPerB) {
			VariableDoublebyteEncoder::write(segmentPerB.first, outputStream);

			VariablebyteEncoder::write(segmentPerB.second.size(), outputStream);
			tbnum += segmentPerB.second.size();
			long previousTS = 0;
			for (auto& aPerB : segmentPerB.second) {
				VariablebyteEncoder::write(static_cast<int>(aPerB.first - previousTS), outputStream);

				previousTS = aPerB.first;
				blist.push_back(aPerB.second);
				
			}
		}
		VariablebyteEncoder::write((lastx), outputStream);
		VariablebyteEncoder::write(lastexist, outputStream);
		if (lastexist == 1) {
			FloatEncoder::write(static_cast<float>(lasty), outputStream);
		}

		huffmanencode(blist, outputStream, frequencyMap);
		string str = outputStream.str();
		bytes.assign(str.begin(), str.end());

	}
	catch (exception& e) {
		cout << e.what() << endl;
	}
	return bytes;
}
bool decompresscompareByT(const decompresSeg& seg1, const decompresSeg& seg2) {
	return seg1.tstart < seg2.tstart;
}

vector<decompresSeg> readbyte(vector<unsigned char>& bytes, int& lastTimeStamp, int& blastexist, float& blasty) {

	stringstream inputStream;
	for (unsigned char c : bytes) {
		inputStream << c;
	}
	vector<decompresSeg>decomprsegVector;
	try {
		int bnuma = VariablebyteEncoder::read(inputStream);
		int bcount = 0;
		for (int i = 0; i < bnuma; i++) {
			double ba = VariableDoublebyteEncoder::read(inputStream);
			
			int bnumbt = VariablebyteEncoder::read(inputStream);
			bcount += bnumbt;
			int bt = 0;
			for (int k = 0; k < bnumbt; k++) {

				bt += VariablebyteEncoder::read(inputStream);
				decomprsegVector.push_back(decompresSeg(bt, ba, 0));
			}
			
		}
		lastTimeStamp = VariablebyteEncoder::read(inputStream);
		blastexist = VariablebyteEncoder::read(inputStream);
		if (blastexist == 1) {
			blasty = FloatEncoder::read(inputStream);
		}
		huffmanDecode(inputStream, bcount, decomprsegVector);
	}
	catch (const exception& e) {
		cout << e.what() << endl;
	}
	sort(decomprsegVector.begin(), decomprsegVector.end(), decompresscompareByT);
	return decomprsegVector;
}