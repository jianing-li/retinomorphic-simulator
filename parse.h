
#include<cstdlib> 
#include "getopt.h"
#include<cstring>
#include<iostream>
#include<iomanip>
using std::string;
using std::setw;
using std::cout;
using std::endl;

extern int HEIGHT = 250;
extern int WIDTH = 400;
extern int CHANNELS = 1;
extern bool USEIMAGESIZE = false;

extern int QUERYTIMES = 1;
extern int THRESHOLD = 2550;
extern int DVSTHRES = 25;
extern bool KEEPRESIDUAL = true;

extern int START_IDX = 1;
extern int END_IDX = 500;

enum filetype{IMG, FORMAT, EMPTY};
extern filetype type = EMPTY;
extern string input_filename = "";
extern string file_format = "";
extern string output_filename = "output.dat";

extern double WIN_SIZE = 9.5;

void show_help();
void parse_command(int argc, char** argv);

void parse_command(int argc, char** argv)
{
	/*
	 * parse command including:
	 * height && width && channels
	 * querytimes && threshold && whether keep residuals
	 *
	 * you can simulate spikes from a single image
	 * or select a folder with a list of images and point the format of the images
	 * maybe a video in the future
	 */
	int c;

	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"width", 			required_argument, 0, 'w'},
			{"height", 			required_argument, 0, 'h'},
			{"channels",  		required_argument, 0, 'c'},
			{"endindex",  		required_argument, 0, 'e'},
			{"threshold",  		required_argument, 0, 't'},
			{"startindex",  	required_argument, 0, 's'},
			{"useimgsize",		no_argument,	   0, 'u'},
			{"help",  			no_argument,	   0, '?'},
			{"dvsthres",		required_argument, 0, 'd'},
			{"input_format", 	required_argument, 0, 'f'},
			{"output_file", 	required_argument, 0, 'o'},
			{"detect_size", 	required_argument, 0, 'z'},
		};

		c = getopt_long(argc, argv, "w:h:c:ue:t:f:o:s:z:d:?", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 'w':
			WIDTH = atoi(optarg);
			break;
		case 'h':
			HEIGHT = atoi(optarg);
			break;
		case 'c':
			CHANNELS = atoi(optarg);
			break;
		case 'u':
			USEIMAGESIZE = true;
			break;
		case 't':
			THRESHOLD = atoi(optarg);
			break;
		case 'e':
			END_IDX = atoi(optarg);
			break;
		case 'd':
			DVSTHRES = atoi(optarg);
			break;
		case 'f':
			file_format = optarg;
			type = FORMAT;
			break;
		case 's':
			START_IDX = atoi(optarg);
			break;
		case 'o':
			output_filename = optarg;
			break;
		case 'z':
			WIN_SIZE = atof(optarg);
			break;
		case '?':
			show_help();
			exit(0);
			break;
		default:
			printf("getopt get undefined character code 0%o\n", c);
			show_help();
			exit(1);
			break;
		}
	}

	if (optind < argc) {
		cout << "not parsed arguments: " << endl;
		while (optind < argc)
			cout << argv[optind++] << endl;
	}
}

void show_help()
{
	/*
	 * help function
	 */

	cout << "Usage of spike simulator: " << endl
		<< "-w or --width:\t\tthe width of the image(s)." << endl
		<< "-h or --height:\t\tthe height of the image(s)." << endl
		<< "-c or --channels:\tthe channels of the image(s)." << endl
		<< "-u or --useimgsize:\tuse the w*h of the images(s)" << endl
		<< "-q or --querytimes:\tthe querytimes of the simulator." << endl
		<< "-t or --threshold:\tthe threshold of the simulator." << endl
		<< "-k or --keepresidual:\twhether to keep residuals in the simulator." << endl
		<< "-f or --input_format:\tformat of a series of input images." << endl
		<< "-o or --output_file:\tname of output file." << endl
		<< "-? or --help:\t\tthis usage." << endl << endl
		<< "The option -i is exclusive with -f, and it's necessary to select -i or -f as input." << endl
		<< "It's not for other options, they have default arguments." << endl
		<< "If you choose -u for -f, the width and height depend on the first image" << endl;
}