#include "getopt.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>

using namespace cv;
using namespace std;
int start_idx = 10, end_idx = 500;
string dirname = "out";
void Generateimage(int* eventcnt, int Width, int Height);
int main(int argc, char** argv)
{
	string outputdir;
	int NumsinImage = 10000;
	int TimeperImage = 1;
	bool UseTime = true;
	string inputfile;
	int Width = 1280, Height = 720;
	double timewin = 5;

	int c;
	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"width",			required_argument, 0, 'w'},
			{"height",			required_argument, 0, 'h'},
			{"num", 			required_argument, 0, 'n'},
			{"frame", 			required_argument, 0, 'f'},
			{"input",  			required_argument, 0, 'i'},
			{"start_idx",		required_argument, 0, 's'},
			{"end_idx", 		required_argument, 0, 'e'},
			{"dirname",			required_argument, 0, 'o'},
			{"timewindow",		required_argument, 0, 't'},
		};
		c = getopt_long(argc, argv, "w:h:n:f:i:s:e:o:t:", long_options, &option_index);
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
		case 'n':
			NumsinImage = atoi(optarg);
			UseTime = false;
			break;
		case 'f':
			TimeperImage = atoi(optarg);
			UseTime = true;
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
		case 't':
			timewin = atoi(optarg);
			break;
		default:
			printf("getopt get undefined character code 0%o\n", c);
			return 0;
			break;
		}
	}

	if (Width <= 0 || Height <= 0)
	{
		cout << "Init image failed." << endl;
		return 0;
	}
	int* eventcnt;
	double* Refertime;
	eventcnt = new int[Width * Height];
	Refertime = new double[Width * Height];

	memset(Refertime, 0, sizeof(double) * Width * Height);

	ifstream EventsFile(inputfile);
	if (!EventsFile)
	{
		cout << "Load file failed: " << inputfile << endl;
		return 0;
	}
	int x, y, p;
	double t;
	char comma;

	if (UseTime)
	{
		bool eofflag = false;
		EventsFile >> x >> y >> t >> p >> comma;
		for (int i = 1;; i += TimeperImage)
		{
			memset(eventcnt, 0, sizeof(int) * Width * Height);
			while(1)
			{
				if (comma != ',' || EventsFile.eof())
				{
					eofflag = true;
					break;
				}
				if (fabs(t - Refertime[x * Height + y]) < timewin)
				{
					if (p == 1)
					{
						eventcnt[x * Height + y] = 1;
					}
					else
					{
						eventcnt[x * Height + y] = -1;
					}
					Refertime[x * Height + y] = t;
				}
				else
				{
					Refertime[x * Height + y] = t;
				}
				
				EventsFile >> x >> y >> t >> p >> comma;
				if (t > i)
				{
					break;
				}
			}
			Generateimage(eventcnt, Width, Height);
			if (eofflag)
			{
				break;
			}
		}

	}
	else
	{
		bool eofflag = false;
		while(1)
		{
			memset(eventcnt, 0, sizeof(int) * Width * Height);
			int posmax = 0, negmax = 0;
			for (int i = 0; i < NumsinImage; i++)
			{
				EventsFile >> x >> y >> t >> p >> comma;

				if (comma != ',' || EventsFile.eof())
				{
					eofflag = true;
					break;
				}
				if (fabs(t - Refertime[x * Height + y]) < timewin)
				{
					if (p == 1)
					{
						eventcnt[x * Height + y] = 1;
					}
					else
					{
						eventcnt[x * Height + y] = -1;
					}
					Refertime[x * Height + y] = t;
				}
				else
				{
					Refertime[x * Height + y] = t;
					i--;
				}
			}
			Generateimage(eventcnt, Width, Height);
			if (eofflag)
			{
				break;
			}
		}
	}
	EventsFile.close();
	return 0;
}

void Generateimage(int* eventcnt, int Width, int Height)
{
	static int cnt = 0;
	Mat out_img = Mat::zeros(Size(Width, Height), CV_8UC3);
	int posmax = 0, negmax = 0;
	for (int i = 0; i < Width; i++)
	{
		for (int j = 0; j < Height; j++)
		{
			int idx = i * Height + j;
			if (eventcnt[idx] == 1)
			{
				out_img.at<Vec3b>(j, i)[0] = 30;
				out_img.at<Vec3b>(j, i)[1] = 30;
				out_img.at<Vec3b>(j, i)[2] = 220;
			}
			else if (eventcnt[idx] == -1)
			{
				out_img.at<Vec3b>(j, i)[0] = 200;
				out_img.at<Vec3b>(j, i)[1] = 30;
				out_img.at<Vec3b>(j, i)[2] = 30;
			}
			else
			{
				out_img.at<Vec3b>(j, i)[0] = 255;
				out_img.at<Vec3b>(j, i)[1] = 255;
				out_img.at<Vec3b>(j, i)[2] = 255;
			}
		}
	}
	char buffer[256];
	sprintf_s(buffer, 256, (dirname + "\\DVSlikeRecon%05d.png").c_str(), cnt + start_idx);
	imwrite(buffer, out_img);
	cout << buffer << " write finished!" << endl;
	cnt++;
}