#include "DVSSimulator.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>
#include <float.h>

static const double eps = 1e-6;
static const double ln2 = log(2);

void DVSSimulator::resetMemory(int width, int height, int channels)
{
	/*
	 * reset the accumulators to fit a new size of image
	 * caution: all value saved will be erased 
	 */
	if(channels != 1)
	{
		std::cout << "Error to reset with channels" << channels << std::endl;
		return;
	}
	if(width <= 0 || height <= 0)
	{
		std::cout << "Bad width or height" << width << " " << height << std::endl;
		return;
	}
	DVSsave.resize(width * height, 0);
	Lastgray.resize(width * height, 0);
}

int DVSSimulator::SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream)
{

	if(channels == 1)
	{
		SimulateoneChannel(img, outstream);
		return 0;
	}
	else
	{
		std::cout << "Error to simulate image with channels " << channels << std::endl
					<< "Please reset the simulator." << std::endl;
		return 1;
	}
}

void DVSSimulator::SimulateoneChannel(cv::Mat& img, std::ofstream& outstream)
{
	static int frames = 0;
	if (frames == 0)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				uint8_t _gray = img.at<uchar>(j, i);							
				Lastgray[i * height	+ j] = DVSsave[i * height + j] = Clamp(_gray, 0, 255);
			}
		}
		frames++;
		return;
	}

	std::vector<Eventrecord> DVSEvents;
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{		
			int idx = i * height + j;
			int value = Clamp(img.at<uchar>(j, i), 0, 255);

			// DVS part
			int DVSpolar = 0;
			int total_change = value - Lastgray[idx];
			if (value - DVSthres >= DVSsave[idx] || value + DVSthres <= DVSsave[idx])
			{
				DVSpolar = value > DVSsave[idx] ? 1 : -1;
				if (total_change == 0)
				{
					int strange_events = (Lastgray[idx] - DVSsave[idx]) * DVSpolar / DVSthres;
					for (int i = 0; i < strange_events; ++i)
					{
						DVSEvents.push_back(0);
					}
				}
				else
				{
					int current_event = DVSsave[idx] + DVSthres * DVSpolar;
					double start_time = (current_event - Lastgray[idx]) / (double)total_change;
					while(start_time >= 0 && start_time <= 1)
					{
						DVSEvents.push_back(start_time);
						start_time += DVSthres * DVSpolar / (double)total_change;
					}
				}
			}
			Lastgray[idx] = value;
		}
	}

	frames++;
	for (const auto dev : DVSEvents)
	{
		outstream << dev.x << " " << dev.y << " " << dev.t << " " << dev.p << ", ";
	}

}
