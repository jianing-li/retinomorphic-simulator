
#include <vector>
#include <iostream>
#include "SImageData.h"
#include "bitbuffer.h"
#include <fstream>
#include <random>

class SImagetoSpike{

private:
	/* defination of image */
	int width;
	int height;
	int channels;					//assert 1 or 3

	/* defination of simulator */
	int query_times_perframe;
	double threshold;
	int DVSthres;
	bool keepResidual;				//reset after pulse?

	double detect_wsize;

	int thres1, thres2;
	double win1, win2;

	::std::vector<double> thresbias;//biased threshold caused by voltage and inductance deviation
	::std::vector<int> accumulateI;	//biased accumulation caused by leakage current
	::std::vector<double> accumulator, accumulator_d;	//accumulator for gray image
	::std::vector<int> accumulatorR, accumulatorG, accumulatorB;
	::std::vector<double> DVStime;
	::std::vector<int> DVSsave;
	::std::vector<int> Usevidar;
	::std::vector<int> Lastgray;

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
	void SimulateoneChannel(SImageData& img, ::std::ofstream& outstream, std::ofstream& DVSoutstream);
	void SimulatethreeChannel(SImageData& img, ::std::ofstream& outstream, std::ofstream& DVSoutstream);

public:
	long long int D_event, S_event;
	SImagetoSpike(int _width = 400, int _height = 250, int _channels = 1, int _query = 1, double _thres = 1020, int _Dt = 50, bool _keep = true, double _dws = 2):
	width(_width), height(_height), channels(_channels), query_times_perframe(_query), threshold(_thres), DVSthres(_Dt), keepResidual(_keep), detect_wsize(_dws)
	{
		if (threshold < detect_wsize * 255)
		{
			std::cout << "detect_wsize too large\n";
			detect_wsize = threshold / 510;
		}
		resetAccumulator(width, height, channels);
		D_event = S_event = 0;
		thres1 = threshold;
		//threshold = 
		thres2 = 50 * thres1;
		win1 = detect_wsize;
		//detect_wsize 
		win2 = 10 * thres2;
	}

	void resetAccumulator(int width, int height, int channels);
	void setSimulator(int query, int thres, bool keep);
	int SimulateSpikeFromImage(SImageData& img, ::std::ofstream& outstream, std::ofstream& DVSoutstream);	
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
