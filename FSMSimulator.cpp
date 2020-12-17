#include "FSMSimulator.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>
#include <float.h>

static const double eps = 1e-6;
static const double ln2 = log(2);

double find_result(double x1, double x2, double y1, double y2, double ans)
{
	if (ans	< 0)
		return 0;
	double delta_x = x2 - x1;
	double max_inte = delta_x * (y1 + y2) / 2;
	
	// no solution
	if(max_inte + eps < ans)
		return -1;

	// y = b
	if(fabs(y1 - y2) < eps && fabs(y1) > eps)
	{
		return ans / y1 + x1;
	}
	
	// y = kx + b
	double k = (y2 - y1) / delta_x;
	double b = y1 - k * x1;
	// kt^2 + 2bt - (kx1^2 + 2bx1 + ans) = 0
	double c = (k * x1 + 2 * b) * x1 + ans;

	if(b*b + k*c < eps || fabs(k) < eps)
	{
		return -1;
	}

	double res1 = (-b + sqrt(b*b + k*c)) / k;
	double res2 = (-b - sqrt(b*b + k*c)) / k;
	
	if(res1 >= x1 && res1 <= x2 && _finite(res1))
		return res1;
	else if(res2 >= x1 && res2 <= x2 && _finite(res2))
		return res2;
	else return -1;
}

void FSMSimulator::resetMemory(int width, int height, int channels)
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

	accumulator.resize(width * height, 0);
	Lastgray.resize(width * height, 0);
}

int FSMSimulator::SimulateSpikeFromImage(cv::Mat& img, std::ofstream& outstream)
{
	/* 
	 * simulate spikes from a given image
	 * 1. resize the image
	 * 2. add the value of each pixel (accumulator for channel==1 && accumulatorR,G,B for channel==3)
	 * 3. compare with the IVSthres and generate spikes! 
	 */

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

void FSMSimulator::SimulateoneChannel(cv::Mat& img, std::ofstream& outstream)
{
	static int frames = 0;
	if (frames == 0)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				uint8_t _gray = img.at<uchar>(j, i);							
				Lastgray[i * height + j] = Clamp(_gray, 0, 255);
			}
		}
		frames++;
		return;
	}

	std::vector<Eventrecord> FSMEvents;
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{		
			int idx = i * height + j;
			int value = Clamp(img.at<uchar>(j, i), 0, 255);
			double res = find_result(0, 1, Lastgray[idx], value, IVSthres - accumulator[idx]);

			if (res < 0)
			{
				accumulator[idx] += (Lastgray[idx] + value) / 2.0;
			}
			else
			{
				Eventrecord Fev(i, j, frames + res, 1);
				FSMEvents.push_back(Fev);
				double endval = Lastgray[idx] + res * (value - Lastgray[idx]);
				accumulator[idx] = (1 - res) * (endval + value) / 2.0;
			}
			Lastgray[idx] = value;
		}
	}
	frames++;

	for (const auto fev : FSMEvents)
	{
		if (!_finite(fev.t))
		{
			int idx = fev.x * height + fev.y;
			accumulator[idx] = 0;
		}
		else
		{
			DVSoutstream << fev.x << " " << fev.y << " " << fev.t << " " << fev.p << ", ";
		}
	}

}
