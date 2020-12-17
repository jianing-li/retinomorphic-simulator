#include "getopt.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stack>
#include "FSMSimulator.h"
#include <float.h>

using namespace cv;
using namespace std;
void Generateimage(int Width, int Height, int totalframes, ifstream& EventsFile);
int clamp(int val);
int start_idx = 10, end_idx = 500;
string dirname = "out";
int main(int argc, char** argv)
{
	string outputdir;
	int totalframes = 50;
	string inputfile;
	int Width = 1280, Height = 720;

	int c;
	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"width",			required_argument, 0, 'w'},
			{"height",			required_argument, 0, 'h'},
			{"frame", 			required_argument, 0, 'f'},
			{"input",  			required_argument, 0, 'i'},
			{"start_idx",		required_argument, 0, 's'},
			{"end_idx", 		required_argument, 0, 'e'},
			{"dirname",			required_argument, 0, 'o'},
		};
		c = getopt_long(argc, argv, "w:h:f:i:s:e:o:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 'w':
			Width = atoi(optarg);
			break;
		case 'h':
			Height = atoi(optarg);
			break;
		case 'f':
			totalframes = atoi(optarg);
			break;
		case 'i':
			inputfile = optarg;
			break;
		case 's':
			start_idx = atoi(optarg);
			break;
		case 'e':
			end_idx = atoi(optarg);
			break;
		case 'o':
			dirname = optarg;
			break;
		default:
			printf("getopt get undefined character code 0%o\n", c);
			return 0;
			break;
		}
	}

	if (Width <= 0 || Height <= 0 || totalframes <= 0)
	{
		cout << "Init image failed." << endl;
		return 0;
	}

	ifstream EventsFile(inputfile);
	if (!EventsFile)
	{
		cout << "Load file failed: " << inputfile << endl;
		return 0;
	}
	Generateimage(Width, Height, totalframes, EventsFile);

	EventsFile.close();
	return 0;
}

void Generateimage(int Width, int Height, int totalframes, ifstream& EventsFile)
{
	static const double eps = 1e-6;
	Mat** out_img;
	out_img = new Mat* [totalframes];
	int accthres = 2550;

	for (int i = 0; i < totalframes; i++)
	{
		out_img[i] = new Mat(Size(Width, Height), CV_8UC1);
		for (int xx = 0; xx < Width; xx++)
			for (int yy = 0; yy < Height; yy++)
				(*out_img[i]).at<uchar>(yy, xx) = 0;
	}
	
	double* Lasttime;
	Lasttime = new double [Width * Height];
	for (int i = 0; i < Width * Height; i++)
	{
		Lasttime[i] = 0;
	}

	int x, y, p;
	double t;
	char comma = ',';
	int curmin = 0;
	char buffer[256];
	int cnt = 0;
	while(1)
	{
		cnt++;
		// EventsFile >> x >> y >> t >> p >> comma;
		EventsFile.getline(buffer, 256, ',');
		int succ = sscanf_s(buffer, "%d %d %lf %d", &x, &y, &t, &p);
		if (succ != 4)
		{
			cout << succ << endl;
			cout << buffer << endl;
			break;
		}
		if (EventsFile.eof() || t >= totalframes)
		{
			cout << x << " " << y << " " << t << " " << totalframes << endl;
			break;
		}
		if (x < 0 || y < 0 || x >= Width || y >= Height || !_finite(t))
		{
			continue;
		}

		int idx = x * Height + y;
		double delta_t = t - Lasttime[idx];
		double val = accthres / (delta_t + eps);
		for (int i = ceil(Lasttime[idx]); i <= t; i++)
		{
			(*out_img[i]).at<uchar>(y, x) = clamp((int)val);
		}
		Lasttime[idx] = t;
	}
	for (int i = 0; i < totalframes; i++)
	{
		sprintf_s(buffer, 256, (dirname +"\\grayRecon%05d.png").c_str(), i + start_idx);
		imwrite(buffer, *out_img[i]);
	}
	
}

int clamp(int val)
{
	if (val > 255)
		val = 255;
	if (val < 0)
		val = 0;
	return val;
}