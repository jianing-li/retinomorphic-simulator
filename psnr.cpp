#include "getopt.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>

using namespace cv;
using namespace std;

double mypsnr(string ref_image, string obj_image)
{
	
	Mat image_ref = cv::imread(ref_image, 0);
	Mat image_obj = cv::imread(obj_image, 0);
	double mse = 0;
	double div = 0;
	int width = image_ref.cols;
	int height = image_ref.rows;
	double psnr = 0;
	for (int v = 0; v < height; v++)
	{
		for (int u = 0; u < width; u++)
		{
			div = image_ref.at<uchar>(v, u) - image_obj.at<uchar>(v, u);
			mse += div*div;

		}
	}
	mse = mse / (width*height);
	if (mse == 0)
	{
		cout << "WOW!" << endl;
		return 100;
	}
	psnr = 10 * log10(255 * 255 / mse);
	return psnr;
}

double myssim(string ref_image, string obj_image)
{
	double C1 = 6.5025, C2 = 58.5225;
	
	cv::Mat image_ref = cv::imread(ref_image, 0);
	cv::Mat image_obj = cv::imread(obj_image, 0);
	int width = image_ref.cols;
	int height = image_ref.rows;
	double mean_x = 0;
	double mean_y = 0;
	double sigma_x = 0;
	double sigma_y = 0;
	double sigma_xy = 0;
	for (int v = 0; v < height; v++)
	{
		for (int u = 0; u < width; u++)
		{
			mean_x += image_ref.at<uchar>(v, u);
			mean_y += image_obj.at<uchar>(v, u);
		}
	}
	mean_x = mean_x / width / height;
	mean_y = mean_y / width / height;
	for (int v = 0; v < height; v++)
	{
		for (int u = 0; u < width; u++)
		{
			sigma_x += (image_ref.at<uchar>(v, u) - mean_x)*(image_ref.at<uchar>(v, u) - mean_x);
			sigma_y += (image_obj.at<uchar>(v, u) - mean_y)*(image_obj.at<uchar>(v, u) - mean_y);
			sigma_xy += (image_ref.at<uchar>(v, u) - mean_x)*(image_obj.at<uchar>(v, u) - mean_y);
		}
	}
	sigma_x = sigma_x / (width*height - 1);
	sigma_y = sigma_y / (width*height - 1);
	sigma_xy = sigma_xy / (width*height - 1);
	double fenzi = (2*mean_x*mean_y + C1) * (2*sigma_xy + C2); 
	double fenmu = (mean_x*mean_x + mean_y*mean_y + C1) * (sigma_x + sigma_y + C2);
	double ssim = fenzi / fenmu;
	return ssim;
}

int main(int argc, char** argv)
{
	int Width = 1280, Height = 720;
	string ref_format;
	string obj_format;
	int start = 0;
	int end = 500;

	int c;
	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"ref",  			required_argument, 0, 'r'},
			{"obj",				required_argument, 0, 'o'},
			{"start",			required_argument, 0, 's'},
			{"end",				required_argument, 0, 'e'},
		};
		c = getopt_long(argc, argv, "r:o:s:e:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 'r':
			ref_format = optarg;
			break;
		case 'o':
			obj_format = optarg;
			break;
		case 's':
			start = atoi(optarg);
			break;
		case 'e':
			end = atoi(optarg);
			break;
		default:
			printf("getopt get undefined character code 0%o\n", c);
			return 0;
			break;
		}
	}

	double max_psnr = -1;
	double min_psnr = 100;
	double max_ssim = -1;
	double min_ssim = 100;
	double avg_psnr = 0;
	double avg_ssim = 0;
	char buffer[256];
	for (int i = start; i < end; i++)
	{
		if (i == 1)
			continue;

		sprintf_s(buffer, 256, ref_format.c_str(), i - 1);
		string ref_img = buffer;
		sprintf_s(buffer, 256, obj_format.c_str(), i);
		string obj_img = buffer;

		double psnr = mypsnr(ref_img, obj_img);
		double ssim = myssim(ref_img, obj_img);

		/*
		if (psnr < min_psnr)
		{
			min_psnr = psnr;
		}
		if (psnr > max_psnr)
		{
			max_psnr = psnr;
		}
		if (ssim < min_ssim)
		{
			min_ssim = ssim;
		}
		if (ssim > max_ssim)
		{
			max_ssim = ssim;
		}
		*/
		avg_psnr += psnr;
		avg_ssim += ssim;
	}
	if (start == 1)
	{
		cout << "ref_images: " + ref_format + " \nobj_images: " + obj_format 
			 << "\navgpsnr: " << avg_psnr / (end - start) << " avgssim: " << avg_ssim / (end - start) << endl;
	}
	else 
	{
		cout << "ref_images: " + ref_format + " \nobj_images: " + obj_format 
			 << "\navgpsnr: " << avg_psnr / (end - start + 1) << " avgssim: " << avg_ssim / (end - start + 1) << endl;
	}

	/*
	cout << "ref_images: " + ref_format + " \nobj_images: " + obj_format + "\npsnr: "
		 << min_psnr << " " << max_psnr << "\nssim: " << min_ssim << " " << max_ssim 
		 << "\navg: " << avg_psnr / (end - start + 1) << " " << avg_ssim / (end - start + 1) << endl;
	*/
	return 0;
}