#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>

class RetinaSimulator{

private:
	/* defination of image */
	int width;
	int height;
	int channels;					//assert 1

	/* defination of simulator */
	double IVSthres;
	int DVSthres;

	double detect_wsize;

	std::vector<double> Accumulator_b, Accumulator_d;	//accumulators
	std::vector<double> Reftime;
	std::vector<int> DVSsave;
	std::vector<int> Lastgray;

	inline int Clamp(int target, int min, int max)
	{
		if(target < min)
			return min;
		else if(target > max)
			return max;
		else return target;
	}

	/*
	 * simulate different types of image, you can set your own here
	 * defination outside the class will not make it inline
	 */
	void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream);

public:
	long long int D_event, I_event;
	RetinaSimulator(int _width = 400, int _height = 250, int _channels = 1, double _It = 1020, int _Dt = 50, double _dws = 2):
	width(_width), height(_height), channels(_channels), IVSthres(_It), DVSthres(_Dt), detect_wsize(_dws)
	{
		if (IVSthres < detect_wsize * 255)
		{
			std::cout << "detect_wsize too large\n";
			detect_wsize = IVSthres / 510;
		}
		resetMemory(width, height, channels);
		D_event = I_event = 0;
	}
	void resetMemory(int width, int height, int channels);
	int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream);	
};

struct Eventrecord
{
	int x;
	int y;
	double t;
	int p;
	bool operator <(const Eventrecord& Er) const
	{
		return t < Er.t;
	}
	Eventrecord(int _x, int _y, double _t, int _p):
	x(_x), y(_y), t(_t), p(_p) {}
};

struct Celerecord
{
	int x;
	int y;
	double t;
	int p;
	int val;
	bool operator <(const Celerecord& Cr) const
	{
		return t < Cr.t;
	}
	Celerecord(int _x, int _y, double _t, int _p, int _val):
	x(_x), y(_y), t(_t), p(_p), val(_val) {}
};
