#include "getopt.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stack>
#include "vidarsimulation.h"
#include <float.h>

using namespace cv;
using namespace std;
int start_idx = 10, end_idx = 500;
string dirname = "out";
void Generateimage(int Width, int Height, int totalframes, ifstream& EventsFile);
int clamp(int val);
double win_size = 9.5;
int DVSthres = 25;
int accthres = 2550;

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
			{"dvsthres",		required_argument, 0, 'd'},
			{"ivsthres",		required_argument, 0, 't'},
			{"detect_size",		required_argument, 0, 'z'},
		};
		c = getopt_long(argc, argv, "w:h:n:f:i:s:e:o:d:t:z:", long_options, &option_index);
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
		case 'd':
			DVSthres = atoi(optarg);
			break;
		case 't':
			accthres = atoi(optarg);
			break;
		case 'z':
			win_size = atof(optarg);
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

	for (int i = 0; i < totalframes; i++)
	{
		out_img[i] = new Mat(Size(Width, Height), CV_8UC1);
		for (int xx = 0; xx < Width; xx++)
			for (int yy = 0; yy < Height; yy++)
				(*out_img[i]).at<uchar>(yy, xx) = 0;
	}
	
	double* Lasttime;
	double* Lastfinaltime;
	int* LastSval;
	stack<Eventrecord>** eventstack;
	eventstack = new stack<Eventrecord>* [Width * Height];
	for (int i = 0; i < Width * Height; i++)
	{
		eventstack[i] = new stack<Eventrecord>();
	}

	Lasttime = new double [Width * Height];
	Lastfinaltime = new double [Width * Height];
	LastSval = new int [Width * Height];
	for (int i = 0; i < Width * Height; i++)
	{
		Lastfinaltime[i] = 0;
		LastSval[i] = -1;
		Lasttime[i] = 0;
	}

	int x, y, p;
	double t;
	char comma = ',';
	int curmin = 0;
	char buffer[256];
	int cnt = 0;
	double symbolt = 0;
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

		if (t > symbolt)
		{
			cout << t << " get!" << endl;
			symbolt += 10;
		}

		// D-event ?
		if (fabs(t - Lasttime[idx]) < win_size)
		{
			Eventrecord devent(x, y, t, p);
			(*eventstack[idx]).push(devent);
			Lasttime[idx] = t;
		}
		// S-event ?             
		else
		{
			double sevent_t = t;
			int sevent_v;

			// D+S format
			if (!(*eventstack[idx]).empty())
			{
				// restore values between D and S 
				Eventrecord tmp = (*eventstack[idx]).top();
				int val;
				if (p == 0)
					val = 255 - accthres / (t - tmp.t); 
				else
					val = accthres / (t - tmp.t);
				val = clamp(val);
				sevent_v = val;
				for (int i = ceil(tmp.t) + eps; i <= t + eps; i++)
				{
					(*out_img[i]).at<uchar>(y, x) = val;
				}
				t = tmp.t;
				p = tmp.p;
				(*eventstack[idx]).pop();

				// restore values between D-events
				double changepersec;
				while(!(*eventstack[idx]).empty())
				{
					tmp = (*eventstack[idx]).top();
					changepersec = DVSthres / (t - tmp.t);
					for (int i = floor(t) + eps; i >= tmp.t - eps; i--)
					{
						if (p == 0)
						{
							(*out_img[i]).at<uchar>(y, x) = clamp(val + (t - i) * changepersec);
						}
						else
						{
							(*out_img[i]).at<uchar>(y, x) = clamp(val - (t - i) * changepersec);
						}
					}
					val = clamp(val + (-2 * p + 1) * DVSthres);
					t = tmp.t;
					p = tmp.p;
					(*eventstack[idx]).pop();
				}

				// restore the value from last S to the first D
				// first null event
				// cannot refer the value
				if (LastSval[idx] < 0)
				{
					changepersec = DVSthres / (t - Lastfinaltime[idx]);
					for (int i = ceil(Lastfinaltime[idx]) + eps; i <= floor(t) + eps; i++)
					{
						if (p == 0)
							(*out_img[i]).at<uchar>(y, x) = clamp(val + (t - i) * changepersec);
						else
							(*out_img[i]).at<uchar>(y, x) = clamp(val - (t - i) * changepersec);
					}
				}
				// last S-event, the p of D-event is not accurate
				else
				{
					if (fabs(t - Lastfinaltime[idx]) > eps)
					{
						changepersec = (val - LastSval[idx]) / (t - Lastfinaltime[idx]);
						for (int i = ceil(Lastfinaltime[idx]) + eps; i <= floor(t) + eps; i++)
						{
							//(*out_img[i]).at<uchar>(y, x) = clamp(val + (i - t) * changepersec);
							(*out_img[i]).at<uchar>(y, x) = LastSval[idx];
						}
					}
				}
			}

			// S S format or null S format
			else
			{
				int val;
				if (p == 0)
					val = 255 - accthres / (t - Lastfinaltime[idx]); 
				else
					val = accthres / (t - Lastfinaltime[idx]);
				val = clamp(val);
				sevent_v = val;
				for (int i = ceil(Lastfinaltime[idx]) + eps; i <= floor(t) + eps; i++)
					(*out_img[i]).at<uchar>(y, x) = val;
			}
			Lastfinaltime[idx] = sevent_t;
			LastSval[idx] = sevent_v;
			Lasttime[idx] = sevent_t; 
		}
	}
	for (int i = 0; i < totalframes; i++)
	{
		sprintf_s(buffer, 256, ("G:\\recon\\Grayrecon\\" + dirname + "\\GrayRecon%05d.png").c_str(), i + start_idx);
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