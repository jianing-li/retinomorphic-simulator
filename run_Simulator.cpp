#include<opencv2/opencv.hpp>
#include<iostream>
#include<fstream>
#include<cstdio>

#include "RetinaSimulator.h"
#include "FSMSimulator.h"
#include "DVSSimulator.h"
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

		RetinaSimulator simulator(WIDTH, HEIGHT, CHANNELS, IVSTHRES, DVSTHRES, WIN_SIZE);
		//FSMSimulator simulator(WIDTH, HEIGHT, CHANNELS, IVSTHRES);
		//DVSSimulator simulator(WIDTH, HEIGHT, CHANNELS, DVSTHRES);
		cout << "WIDTH:" << WIDTH << " HEIGHT:" << HEIGHT << endl;

		std::ofstream outstream;
		outstream.open(output_filename, std::ios::binary);

		Size ResizeSize = Size(WIDTH, HEIGHT);

		while(1)
		{
			//resize(img, img, ResizeSize,0, 0, INTER_AREA);
			if (!USEIMAGESIZE)
			{
				resize(img, img, ResizeSize);
			}			
			simulator.SimulateEventFromImage(img, outstream);

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
		cout << "D-event: " << simulator.D_event << "I_event: " << simulator.I_event << endl; 
		//cout << "I_event: " << simulator.I_event << endl; 
		//cout << "D_event: " << simulator.D_event << endl;
		cout << file_idx - START_IDX << " image(s) is(are) simulated in total." << endl;
		return 0;
	}

	cout << "Something strange occurs......" << endl;
	return 0;
}