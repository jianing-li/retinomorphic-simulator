#include "getopt.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	int Width = 1280, Height = 720;
	string inputformat;
	string dformat;
	string outformat;
	int start_idx = 555;
	int end_idx = 928;

	int c;
	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"input",  			required_argument, 0, 'i'},
			{"dfile",			required_argument, 0, 'd'},
			{"output",			required_argument, 0, 'o'},
			{"start_idx",		required_argument, 0, 's'},
			{"end_idx", 		required_argument, 0, 'e'},
		};
		c = getopt_long(argc, argv, "i:d:o:s:e:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 'i':
			inputformat = optarg;
			break;
		case 'd':
			dformat = optarg;
			break;
		case 'o':
			outformat = optarg;
			break;
		case 's':
			start_idx = atoi(optarg);
			break;
		case 'e':
			end_idx = atoi(optarg);
			break;
		default:
			printf("getopt get undefined character code 0%o\n", c);
			return 0;
			break;
		}
	}

	Mat graymat, dmat;
	char buffer[256];
	for (int i = start_idx; i < end_idx; ++i)
	{
		sprintf_s(buffer, 256, inputformat.c_str(), i);
		graymat = imread(buffer);
		sprintf_s(buffer, 256, dformat.c_str(), i);
		dmat = imread(buffer);
		if (!graymat.data || !dmat.data)
		{
			cout << "Finish at " << i << endl;
		}
		for (int i = 0; i < graymat.cols; i++)
		{
			for (int j = 0; j < graymat.rows; ++j)
			{
				if (dmat.at<Vec3b>(j, i)[0] != 255)
				{
					graymat.at<Vec3b>(j, i)[0] = dmat.at<Vec3b>(j, i)[0];
					graymat.at<Vec3b>(j, i)[1] = dmat.at<Vec3b>(j, i)[1];
					graymat.at<Vec3b>(j, i)[2] = dmat.at<Vec3b>(j, i)[2];
				}
			}
		}
		sprintf_s(buffer, 256, outformat.c_str(), i);
		imwrite(buffer, graymat);
		cout << i << " ";
	}
	return 0;
}