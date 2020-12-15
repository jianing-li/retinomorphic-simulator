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
	string dfileformat;
	string mergeformat;
	string outformat;
	string originformat;
	int start_idx = 555;
	int end_idx = 928;

	int c;
	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"inputformat",  	required_argument, 0, 'i'},
			{"dfileformat",		required_argument, 0, 'd'},
			{"outformat",		required_argument, 0, 'f'},
			{"originformat",	required_argument, 0, 'o'},
			{"start_idx",		required_argument, 0, 's'},
			{"end_idx", 		required_argument, 0, 'e'},
			{"mergeformat",		required_argument, 0, 'm'},
		};
		c = getopt_long(argc, argv, "i:d:m:o:s:e:f:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 'i':
			inputformat = optarg;
			break;
		case 'd':
			dfileformat = optarg;
			break;
		case 'm':
			mergeformat = optarg;
			break;
		case 'f':
			outformat = optarg;
			break;
		case 'o':
			originformat = optarg;
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

	Mat graymat, dmat, mergemat, originmat;
	Mat outmat(Height, Width, CV_8UC3, Scalar(0,0,0));
	char buffer[256];
	Point p1 = Point(650, 60);
	Point p2 = Point(850, 60);
	Point p3 = Point(700, 60);
	Point p4 = Point(880, 60);
	for (int i = start_idx; i < end_idx; ++i)
	{
		sprintf_s(buffer, 256, inputformat.c_str(), i);
		graymat = imread(buffer);
		sprintf_s(buffer, 256, dfileformat.c_str(), i);
		dmat = imread(buffer);
		sprintf_s(buffer, 256, mergeformat.c_str(), i);
		mergemat = imread(buffer);
		sprintf_s(buffer, 256, originformat.c_str(), i);
		originmat = imread(buffer);

		if (!graymat.data || !dmat.data || !mergemat.data || !originmat.data)
		{
			cout << !graymat.data << !dmat.data << !mergemat.data << !originmat.data << endl;
			cout << "Finish at " << i << endl;
		}
		putText(graymat, "Texture Reconstruction", p1, FONT_HERSHEY_TRIPLEX, 1.5, Scalar(15, 230, 230), 2, LINE_AA);
		putText(dmat, "Dynamic Events", p2, FONT_HERSHEY_TRIPLEX, 1.5, Scalar(15, 230, 230), 2, LINE_AA);
		putText(mergemat, "Texture and Dynamic", p3, FONT_HERSHEY_TRIPLEX, 1.5, Scalar(15, 230, 230), 2, LINE_AA);
		putText(originmat, "Original Image", p4, FONT_HERSHEY_TRIPLEX, 1.5, Scalar(15, 230, 230), 2, LINE_AA);

		resize(graymat, graymat, Size(640, 360));
		resize(dmat, dmat, Size(640, 360));
		resize(mergemat, mergemat, Size(640, 360));
		resize(originmat, originmat, Size(640, 360));

		for (int k = 0; k < 640; k++)
		{
			for (int j = 0; j < 360; j++)
			{
				outmat.at<Vec3b>(j, k)[0] = originmat.at<Vec3b>(j, k)[0]; outmat.at<Vec3b>(j, k)[1] = originmat.at<Vec3b>(j, k)[1]; outmat.at<Vec3b>(j, k)[2] = originmat.at<Vec3b>(j, k)[2];
				outmat.at<Vec3b>(j+360, k)[0] = graymat.at<Vec3b>(j, k)[0]; outmat.at<Vec3b>(j+360, k)[1] = graymat.at<Vec3b>(j, k)[1]; outmat.at<Vec3b>(j+360, k)[2] = graymat.at<Vec3b>(j, k)[2];
				outmat.at<Vec3b>(j, k+640)[0] = dmat.at<Vec3b>(j, k)[0]; outmat.at<Vec3b>(j, k+640)[1] = dmat.at<Vec3b>(j, k)[1]; outmat.at<Vec3b>(j, k+640)[2] = dmat.at<Vec3b>(j, k)[2];
				outmat.at<Vec3b>(j+360, k+640)[0] = mergemat.at<Vec3b>(j, k)[0]; outmat.at<Vec3b>(j+360, k+640)[1] = mergemat.at<Vec3b>(j, k)[1]; outmat.at<Vec3b>(j+360, k+640)[2] = mergemat.at<Vec3b>(j, k)[2];
			}
		}
		sprintf_s(buffer, 256, outformat.c_str(), i + 1 - start_idx);
		imwrite(buffer, outmat);
		cout << i << " ";
	}
	return 0;
}