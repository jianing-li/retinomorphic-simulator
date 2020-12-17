#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>

class FSMSimulator{

private:
	/* defination of image */
	int width;
	int height;
	int channels;					//assert 1

	/* defination of simulator */
	double IVSthres;

	std::vector<double> accumulator;	
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
	long long int I_event;
	FSMSimulator(int _width = 400, int _height = 250, int _channels = 1, double _It = 1020):
	width(_width), height(_height), channels(_channels), IVSthres(_It)
	{
		resetMemory(width, height, channels);
		I_event = 0;
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
