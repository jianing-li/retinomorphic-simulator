#include "vidarsimulation.h"
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

void SImagetoSpike::setSimulator(int query, int thres, bool keep)
{
	/* 
	 * reset arguments of simulator
	 * in case the initalized ones are not suitable
	 */
	if(query <= 0 || thres <= 0)
	{
		::std::cout << "Bad arguments for the simulator!" << ::std::endl;
		return;
	}
	query_times_perframe = query;
	threshold = thres;
	keepResidual = keep;
}

void SImagetoSpike::resetAccumulator(int width, int height, int channels)
{
	/*
	 * reset the accumulators to fit a new size of image
	 * caution: all value saved will be erased 
	 */
	if(channels != 1 && channels != 3)
	{
		::std::cout << "Error to reset with channels" << channels << ::std::endl;
		return;
	}
	if(width <= 0 || height <= 0)
	{
		::std::cout << "Bad width or height" << width << " " << height << ::std::endl;
		return;
	}
	if(channels == 1)
	{
		accumulatorR.resize(0);
		accumulatorG.resize(0);
		accumulatorB.resize(0);
		accumulator.resize(width * height, 0);
		accumulator_d.resize(width * height, 0);
	}
	if(channels == 3)
	{
		accumulatorR.resize(width * height, 0);
		accumulatorG.resize(width * height, 0);
		accumulatorB.resize(width * height, 0);
		accumulator.resize(0);
		accumulator_d.resize(0);
	}
	accumulateI.resize(0);
	thresbias.resize(0);
	DVStime.resize(width * height, 0);
	DVSsave.resize(width * height, 0);
	Usevidar.resize(width * height, 0);
	Lastgray.resize(width * height, 0);
}

int SImagetoSpike::SimulateSpikeFromImage(SImageData& img, ::std::ofstream& outstream, std::ofstream& DVSoutstream)
{
	/* 
	 * simulate spikes from a given image
	 * 1. resize the image
	 * 2. add the value of each pixel (accumulator for channel==1 && accumulatorR,G,B for channel==3)
	 * 3. compare with the threshold and generate spikes! 
	 */

	SImageData r_img;
	ImageResize(img, r_img, width, height);
	if(channels == 1)
	{
		SimulateoneChannel(r_img, outstream, DVSoutstream);
		return 0;
	}

	else if(channels == 3)
	{
		SimulatethreeChannel(r_img, outstream, DVSoutstream);
		return 0;
	}

	else
	{
		::std::cout << "Error to simulate image with channels " << channels << ::std::endl
					<< "Please reset the simulator." << ::std::endl;
		return 1;
	}
}

void SImagetoSpike::SimulateoneChannel(SImageData& img, ::std::ofstream& outstream, std::ofstream& DVSoutstream)
{
	static int frames = 0;
	if (frames == 0)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				SColor  C = img.GetPixel(i, j);
				uint8 _gray = C.LumaU8();								
				Lastgray[i * height	+ j] = DVSsave[i * height + j] = Clamp(_gray, 0, 255);
				DVStime[i * height + j] = 0;
			}
		}
		frames++;
		return;
	}

	bit_buffer grayBits;
	std::vector<Eventrecord> ATISEvents;
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{		
			int idx = i * height + j;
			int value = Clamp(img.GetPixel(i, j).LumaU8(), 0, 255);

			// IVS part
			double ans1 = find_result(0, 1, Lastgray[idx], value, threshold - accumulator[idx]);
			double ans2 = find_result(0, 1, 255 - Lastgray[idx], 255 - value, threshold - accumulator_d[idx]);
			double ans = ans1;
			int IVSpolar = 1;
			if (ans < 0 || (ans2 < ans && ans2 >= 0))
			{
				ans = ans2;
				IVSpolar = 0;
			}

			// DVS part
			int DVSpolar = 0;
			std::vector<double> DVS_times;
			int total_change = value - Lastgray[idx];
			if (value - DVSthres >= DVSsave[idx] || value + DVSthres <= DVSsave[idx])
			{
				DVSpolar = value > DVSsave[idx] ? 1 : -1;
				if (total_change == 0)
				{
					int strange_events = (Lastgray[idx] - DVSsave[idx]) * DVSpolar / DVSthres;
					for (int i = 0; i < strange_events; ++i)
					{
						DVS_times.push_back(0);
					}
				}
				else
				{
					int current_event = DVSsave[idx] + DVSthres * DVSpolar;
					double start_time = (current_event - Lastgray[idx]) / (double)total_change;
					while(start_time >= 0 && start_time <= 1)
					{
						DVS_times.push_back(start_time);
						start_time += DVSthres * DVSpolar / (double)total_change;
					}
				}
			}

			// IVS failed and DVS failed
			if (ans < 0 && DVSpolar == 0)
			{
				// integrating
				accumulator[idx] += (Lastgray[idx] + value) / 2.0;
				accumulator_d[idx] += 255 - (Lastgray[idx] + value) / 2.0;
			}
			// IVS succeeded and DVS failed
			if (ans >= 0 && DVSpolar == 0)
			{
				Eventrecord Sev(i, j, frames + ans, IVSpolar);
				ATISEvents.push_back(Sev);
				DVStime[idx] = frames + ans;
				S_event++;
				double end_val = Lastgray[idx] + ans * total_change;
				accumulator[idx] = (end_val + value) * (1 - ans) / 2.0;
				accumulator_d[idx] = (510 - end_val - value) * (1 - ans) / 2.0;
			}
			// IVS failed and DVS succeeded
			if (ans < 0 && DVSpolar != 0)
			{
				accumulator[idx] += (Lastgray[idx] + value) / 2.0;
				accumulator_d[idx] += 255 - (Lastgray[idx] + value) / 2.0;
				for (const auto curr_t : DVS_times)
				{
					if (fabs(frames + curr_t - DVStime[idx]) < detect_wsize)
					{
						Eventrecord Dev(i, j, frames + curr_t, (DVSpolar + 1) / 2);
						ATISEvents.push_back(Dev);
						D_event++;
						DVSsave[idx] += DVSthres * DVSpolar;
						DVStime[idx] = frames + curr_t;
						double end_val = DVSsave[idx];
						accumulator[idx] = (end_val + value) * (1 - curr_t) / 2.0;
						accumulator_d[idx] = (510 - end_val - value) * (1 - curr_t) / 2.0;
					}
					else
					{
						DVSsave[idx] += DVSthres * DVSpolar;
					}
				}
			}
			// IVS succeeded and DVS succeeded
			if (ans >= 0 && DVSpolar != 0)
			{
				Eventrecord Sev(i, j, frames + ans, IVSpolar);
				ATISEvents.push_back(Sev);
				DVStime[idx] = frames + ans;
				S_event++;
				double end_val = Lastgray[idx] + ans * total_change;
				accumulator[idx] = (end_val + value) * (1 - ans) / 2.0;
				accumulator_d[idx] = (510 - end_val - value) * (1 - ans) / 2.0;
				for (const auto curr_t : DVS_times)
				{
					if (curr_t < ans)
					{
						// these event must be ignored
						// in case violate the machenism
						DVSsave[idx] += DVSthres * DVSpolar;
					}
					else if (fabs(frames + curr_t - DVStime[idx]) < detect_wsize)
					{
						Eventrecord Dev(i, j, frames + curr_t, (DVSpolar + 1) / 2);
						ATISEvents.push_back(Dev);
						D_event++;
						DVSsave[idx] += DVSthres * DVSpolar;
						DVStime[idx] = frames + curr_t;
						end_val = DVSsave[idx];
						accumulator[idx] = (end_val + value) * (1 - curr_t) / 2.0;
						accumulator_d[idx] = (510 - end_val - value) * (1 - curr_t) / 2.0;
					}
					else
					{
						DVSsave[idx] += DVSthres * DVSpolar;
					}
				}
			}
			Lastgray[idx] = value;
		}
	}

	frames++;
	//sort(ATISEvents.begin(), ATISEvents.end());
	for (const auto aev : ATISEvents)
	{
		if (!_finite(aev.t))
		{
			int idx = aev.x * height + aev.y;
			accumulator[idx] = accumulator_d[idx] = 0;
			DVSsave[idx] = Clamp(img.GetPixel(aev.x, aev.y).LumaU8(), 0, 255);
			DVStime[idx] = frames;
		}
		else
		{
			DVSoutstream << aev.x << " " << aev.y << " " << aev.t << " " << aev.p << ", ";
		}
	}

}

void SImagetoSpike::SimulatethreeChannel(SImageData& img, ::std::ofstream& outstream, std::ofstream& DVSoutstream)
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			SColor  C = img.GetPixel(i, j);				
			accumulatorR[i * height + j] += C.R; 
			accumulatorG[i * height + j] += C.G; 
			accumulatorB[i * height + j] += C.B; 
		}
	}
	for (int k = 0; k < query_times_perframe; k++)
	{
		bit_buffer RedBits;
		bit_buffer GreenBits;
		bit_buffer BlueBits;
		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i += 8)
			{
				for (int e = 7; e >= 0; e--)
				{
					if(i + e >= width)
						continue;
					int idx = (i + e) * height + j;
					if (accumulatorR[idx] > threshold)
					{
						RedBits.write_bits(true, 1);
						if (keepResidual) accumulatorR[idx] -= threshold;
						else accumulatorR[idx] = 0;
					}
					else RedBits.write_bits(false, 1);

					if (accumulatorG[idx] > threshold)
					{
						GreenBits.write_bits(true, 1);
						if (keepResidual) accumulatorG[idx] -= threshold;
						else accumulatorG[idx] = 0;
					}
					else GreenBits.write_bits(false, 1);

					if (accumulatorB[idx] > threshold)
					{
						BlueBits.write_bits(true, 1);
						if (keepResidual) accumulatorB[idx] -= threshold;
						else accumulatorB[idx] = 0;
					}
						else BlueBits.write_bits(false, 1);
				}
			}
		}

		//write into bitstream
		for (auto& byte : RedBits.get_bytes())
			outstream << (uint8)byte;
		for (auto& byte : GreenBits.get_bytes())
			outstream << (uint8)byte;
		for (auto& byte : BlueBits.get_bytes())
			outstream << (uint8)byte;
	}
}

bit_buffer::bit_buffer(const size_t size) :	pos_(0), bit_index_(0)
{
	this->buffer_.reserve(size);
}

bit_buffer::~bit_buffer() {}