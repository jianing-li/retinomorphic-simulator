#include<opencv2/opencv.hpp>
#include<iostream>
#include<fstream>
#include<cstdio>

#include "vidarsimulation.h"
#include "parse.h"

using namespace cv;

int main(int argc, char** argv)
{
	parse_command(argc, argv);
	if(type == EMPTY)
	{
		cout << "No input file, please select an image or a directory." << endl;
		show_help();
		return 0;
	}

	static Mat img;
	if(type == FORMAT)
	{
		/*
		 * simulate spikes of a sequence of images
		 * initalize simulator, continuously load images and simulate
		 */
		int file_idx = START_IDX;
		char buffer[256];
		int symbolnum = 0;
		for (int i = 0; i < file_format.size(); i++)
		{
			if(file_format[i] == '%')
				symbolnum++;
			if(symbolnum >= 2)
			{
				cout << "Too many % in the format, fail to parse." << endl;
				return 0;
			}

		}
		if(symbolnum == 0)
		{
			cout << "Warning: no % in format, simulate may fail." << endl;
		}
		sprintf_s(buffer, 256, file_format.c_str(), file_idx);
		
		string input_file = buffer;
		img = imread(input_file);
		if(img.empty())
		{
			cout << "Failed to load file: " << input_file << endl;
			return 0;
		}
		if(USEIMAGESIZE)
		{
			WIDTH = img.cols;
			HEIGHT = img.rows;
		}

		SImageData simg;
		ImageInit(simg, WIDTH, HEIGHT);

		SImagetoSpike simulator(WIDTH, HEIGHT, CHANNELS, QUERYTIMES, THRESHOLD, DVSTHRES, KEEPRESIDUAL, WIN_SIZE);
		cout << "WIDTH:" << WIDTH << " HEIGHT:" << HEIGHT << endl;

		::std::ofstream outstream;
		outstream.open("tmp.dat", ::std::ios::binary);
		::std::ofstream DVSoutstream;
		DVSoutstream.open(output_filename, ::std::ios::binary);

		Size ResizeSize = Size(WIDTH, HEIGHT);

		while(1)
		{
			//resize(img, img, ResizeSize,0, 0, INTER_AREA);
			if (!USEIMAGESIZE)
			{
				resize(img, img, ResizeSize);
			}			
			for(int i = 0; i < WIDTH; i++)
			{
				for(int j = 0; j < HEIGHT; j++)
				{
					SColor& targetC = simg.GetPixel(i, j);
					targetC.R = img.at<Vec3b>(j, i)[2];
					targetC.G = img.at<Vec3b>(j, i)[1];
					targetC.B = img.at<Vec3b>(j, i)[0];
				}
			}
			simulator.SimulateSpikeFromImage(simg, outstream, DVSoutstream);

			// load next image, break when empty
			file_idx++;
			sprintf_s(buffer, 256, file_format.c_str(), file_idx);
			input_file = buffer;
			img = imread(input_file);
			if(img.empty() || file_idx == END_IDX)
			{
				cout << "Simulate end here." << endl;
				break;
			}

			if (file_idx % 200 == 0)
			{
				cout << file_idx << " frames finished!" << endl;
			}
		}
		outstream.close();
		cout << "D-event: " << simulator.D_event << "S_event: " << simulator.S_event << endl; 
		cout << file_idx - START_IDX << " image(s) is(are) simulated in total." << endl;
		return 0;
	}

	cout << "Something strange occurs......" << endl;
	return 0;
}