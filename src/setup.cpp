#include "main.h"
#include "util.h"
#include "setup.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <getopt.h> // for getopt_long
#include <iterator>
using namespace std;

// default constructor of setup class
Setup::Setup(int argc, char** argv)
{
#if(DEBUG == 1)
    cout << "Setup constructor method called." << endl;
#endif

	// copy command line arguments
	this -> argc = argc;
	this -> argv = argv;

	// Initialise global variables
	args = {0};

	// set version
	args.version = VERSION;

	// define instruments
	setDevices();

	// parse command line
	parseCmdLine();

	// handle --list option
	if ( !args.listFilename.empty() ) {
		parseListFile();
	}

	setOutputFilename();

	// print header
	//displayHeader();

    // start timer
    startTimer();
}

// default destructor
Setup::~Setup()
{
#if(DEBUG == 1)
	cout << "Setup destructor method called." << endl;
#endif

	// stop timer
	stopTimer();
}

// error handler for setup class
void Setup::errorHandler(ReturnCodes ret, const string& msg)
{
	string message;

	switch (ret) {
	case CodeOk:
		throw return_exception("Ok.");
		break;

	case CodeError:
		throw logic_error("Error.");
		break;

	case CodeCmdParseError:
		throw logic_error("Command line parse error.");
		break;

	case CodeVersionRequested:
		displayHeader();
		//cout << "Version: " << args.version << endl;
		throw return_exception();
		break;

	case CodeHelpRequested:
		displayHelp();
		throw return_exception();
		break;

	case CodeInfoRequested:
		displayInfo();
		throw return_exception();
		break;

	case CodeFileNotFound:
		throw logic_error("File '" + msg + "' not found.");
		break;

	case CodeFileCouldNotOpen:
		throw logic_error("Could not open '" + msg + "'.");
		break;

	case CodeFileTypeNotValid:
		throw logic_error("File type '" + msg + "' not supported.");
		break;

	case CodeFileNotValid:
		throw logic_error("File '" + msg + "' not valid.");
		break;

	case CodeEmptyListFile:
		throw logic_error("List file empty.");
		break;

	case CodeDataUnknown:
		throw logic_error(msg + ": Unknown data type.");
		break;

	case CodeCtimeError:
		throw logic_error(msg + ": ctime error.");
		break;

	case CodeODBCerror:
		throw logic_error("ODBC error.");
		break;

	case CodeSqliteError:
		throw logic_error("Sqlite3: " + msg);
		break;

	default:
		throw logic_error("Unknown error.");
		break;
	}
}

// parse command line options
void Setup::parseCmdLine()
{
	// no options and args
	if (argc == 1) {
		errorHandler(CodeHelpRequested);
	}

    const struct option longOpts[] = {
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'V' },
        { "verbose", no_argument, NULL, 'v' },
        { "info", no_argument, NULL, 'i' },
        { "output", required_argument, NULL, 'o' },
        { "file", required_argument, NULL, 'f' },
		{ "timer", no_argument, NULL, 't' },
		{ "smooth", required_argument, NULL, 's'},
        { NULL, 0, NULL, 0 }
    };

    const char* optString = "hVvio:f:ts:";
    int opt = 0;
    int longIndex = 0;

	do {
    	opt = getopt_long( argc, argv, optString, longOpts, &longIndex );

        switch( opt ) {
		case 'h':
			errorHandler(CodeHelpRequested);
			break;

		case 'V':
			errorHandler(CodeVersionRequested);
			break;

		case 'v':
			args.isVerbose = true;
			break;

		case 'i':
			errorHandler(CodeInfoRequested);
			break;

		case 'o':
			args.outputFilename = optarg;
			break;

		case 'f':
			args.listFilename = optarg;
			break;

		case 't':
			args.isTimer = true;
			break;

		case 's':
			args.smooth = atoi(optarg);
			break;

		case '?':
			errorHandler(CodeCmdParseError);
			break;

		case ':':
            errorHandler(CodeCmdParseError);
            break;

		default:
			break;
		}

	} while (opt != -1);

	// get remaining command line arguments (not options)
	if (optind < argc) {
        args.fileNames.assign((argv + optind), (argv + argc));
	}
}

// display header
void Setup::displayHeader()
{
	int length = args.version.length();

	cout << "\
+-----------------------------------------------------------------------+\n\
| Author      : Alexander Pohl                                          |\n\
| License     : MIT                                                     |\n\
| Version     : " << args.version << setw(56 - length) << " " << "|\n\
| Description : Converter for potentiostat data into a SQLite database  |\n\
+-----------------------------------------------------------------------+\n\
" << endl;
}


// display help
void Setup::displayHelp()
{
	cout << endl << "Usage: " << argv[0] << " [options] file1 [file2] [file3] ..." << endl << endl;
	cout << "\
  -h --help              Print this help message\n\
  -V --version           Print version info\n\
  -v --verbose           Be more verbose\n\
  -i --info              Show supported instruments\n\
  -t --timer             Benchmark program run time\n\
  -o --output [file]     Output file name, optional\n\
  -f --file [file]       Read files from file, optional\n\
  -s --smooth [1,2,3,4]  Smooth current and voltage data" << endl << endl;
}

// display info
void Setup::displayInfo()
{
    cout << "\
+-------------------------------------+\n\
|    *** Supported Instruments ***    |\n\
+-------------------------------------+\n\
" << endl;

	map<string, string>::const_iterator it = args.device.begin();

	while (it != args.device.end()) {
		cout << "  " << (it -> second);
		cout <<  " (*." << (it -> first) << ")" << endl;
		it++;
	}
	cout << endl;
}

// setup instruments
void Setup::setDevices()
{
	// instruments
	args.device.insert(make_pair("res", "Arbin BT2000"));
	args.device.insert(make_pair("mpt", "Biologic VMP3 text"));
	args.device.insert(make_pair("DTA", "Gamry Interface 1000"));
	args.device.insert(make_pair("idf", "Ivium CompactStat"));
	args.device.insert(make_pair("txt", "Zahner IM6"));
	args.device.insert(make_pair("mpr", "Biologic VMP3 binary"));

	// plot data types
	args.plots.insert(make_pair(CodeIVCurve, "IV curve"));
	args.plots.insert(make_pair(CodeOpenCircuitPotential, "Open circuit potential"));
	args.plots.insert(make_pair(CodeImpedanceSpectrum, "Impedance spectrum"));
	args.plots.insert(make_pair(CodeCyclicVoltammetry, "Cyclic Voltammetry"));
}

// parse list of filenames given with (--list) option
void Setup::parseListFile()
{
	ifstream reader(args.listFilename);
	string line;
	vector<string> tokens;

	// test if file exists and can be opened
	if ( !reader.good() ) {
		errorHandler(CodeFileNotFound, args.listFilename);
	}
	
	// init vector array of filenames (delete cmd args)
	args.fileNames.clear();

	while ( !reader.eof() ) {
		getline( reader, line );
		
		// ignore characters after comment (!)
		string::size_type pos = line.find("!", 0);
		line = line.substr(0, pos);		

		// split line
		util::tokenize(line, tokens, " \t");
	}
	
	args.fileNames.swap(tokens);
	
	if (args.fileNames.size() == 0) {
		errorHandler(CodeEmptyListFile);
	}

#if(DEBUG == 1)
	copy( args.fileNames.begin(), args.fileNames.end(), ostream_iterator<string>(cout, ", "));
	cout << endl;
#endif

	reader.close();
}

// set output filename in args
void Setup::setOutputFilename()
{
	// --output option not given, use name of first file
	if ( args.outputFilename.empty() ) {
		args.outputFilename = args.fileNames.at(0).substr(
				0, args.fileNames.at(0).find_last_of('.'))+".sqlite";
	} else {
	// use --output option, add extension
		args.outputFilename = args.outputFilename.substr(
		0, args.outputFilename.find_last_of('.'))+".sqlite";
	}
}

global_t Setup::getArguments()
{
	return args;
}

// benchmark start timer
void Setup::startTimer()
{
	clockStart = chrono::steady_clock::now();
}

// benchmark stop timer
void Setup::stopTimer()
{
	chrono::steady_clock::duration elapsed;
	double secs, recsPerSec;

	elapsed = chrono::steady_clock::now() - clockStart;
	secs = chrono::duration <double, milli> (elapsed).count() / 1000.0;
	recsPerSec = (double) args.recordsSum / secs;

	if ( args.isTimer ) {
		cout << "Run time " <<
				fixed << setprecision(1) << secs << " seconds, " <<
				fixed << setprecision(0) << recsPerSec << " records per second." <<
				endl;
	}
}
