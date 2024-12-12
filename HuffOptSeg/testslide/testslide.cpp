#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include <iomanip>
#include <string.h>
#include "segmentset.h"


void loadRealData(const string& filePath, vector<string>& timeSeriesFileNames);

using namespace std;
int main()
{
	string realTimeSeriesPath = "time_series/";
	vector<string> timeSeriesFileNames;
	/***********Reading Parameters************/
	int method = (int)3;
	if (method < 0)
	{
		cout << "Error in the method choice!" << endl;
		return 0;
	}
	double sigma = 0;
	double initialData;
	string errorName = "time_series/error.txt";
	FILE* fp;
	fp = fopen(errorName.c_str(), "r");
	if (fp == NULL)
	{
		exit(1);
	}
	while (!feof(fp))
	{
		try
		{
			int f = fscanf(fp, "%lf", &initialData);
			sigma = initialData;
		}
		catch (exception e)
		{
			exit(1);
		}
	}
	fclose(fp);


	loadRealData(realTimeSeriesPath, timeSeriesFileNames);
	int i;
	string szFileName;
	cout << timeSeriesFileNames.size() << endl;

	for (i = 0; i < (int)(timeSeriesFileNames.size()); i++) {
		szFileName = realTimeSeriesPath + timeSeriesFileNames[i];
		cout << realTimeSeriesPath << endl;
		cout << timeSeriesFileNames[i] <<" " << i << endl;
		ifstream streamFile3(szFileName.c_str());

		for (double k = 0.1; k <= 0.1; k = k + 0.01)
		{

			for (int j = 0; j < 1; j++)
			{
				HuffOptSeg(streamFile3, sigma, szFileName, timeSeriesFileNames[i]);

			}
		}
		streamFile3.close();

	}

	return 1;
}


void loadRealData(const string& filePath, vector<string>& timeSeriesFileNames) {

	FILE* fpIn;
	string szFileName;
	char szLine[1024];

	szFileName = filePath + "files.txt";
	if ((fpIn = fopen(szFileName.c_str(), "r")) == NULL) {
		printf("Cannot open read %s", szFileName.c_str());
		exit(0);
	}

	while (fscanf(fpIn, "%s", szLine) == 1) {
		timeSeriesFileNames.push_back(szLine);
	}

	fclose(fpIn);
}

