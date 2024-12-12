#pragma once
//线段结构体
typedef struct GenerateSeg
{
	double vstartup;
	double vstartlow;
	long timestart;

	GenerateSeg() {};
	GenerateSeg(double vstartup, double vstartlow, long timestart) :
		 vstartup(vstartup), vstartlow(vstartlow),timestart(timestart){}
}GenerateSeg;

typedef struct decompresSeg
{
	int tstart;
	double avra;
	double avrv;
	decompresSeg() {};
	decompresSeg(int tstart, double avra, double avrv) :
		tstart(tstart), avra(avra), avrv(avrv) {}
	void setB(double value) { avrv = value; }
}decompresSeg;
typedef struct SPSegment {
	long initTimestamp;
	double aMin;
	double aMax;
	double b;
	SPSegment() {};
	SPSegment(long initTimestamp, double aMin, double aMax, double b) :
		initTimestamp(initTimestamp), aMin(aMin), aMax(aMax), b(b) {}
	void setAMin(double aMin) {
		this->aMin = aMin;
	}
	void setAMax(double aMax) {
		this->aMax = aMax;
	}
}SPSegment;