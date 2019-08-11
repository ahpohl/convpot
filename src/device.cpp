#include <iostream>
#include <iomanip>
#include <fstream>

#include "main.h"
#include "util.hpp"
#include "device.hpp"

using namespace std;

// device constructor
Device::Device(int argc, char** argv) : Setup(argc, argv)
{
#if(DEBUG == 1)
    cout << "Device constructor method called." << endl;
#endif

    // run device methods
	readDataFiles();
	setGlobalDevice();
	calculateCycles();

	if (args.verbosity >= 3) {
		printCalcRecords();
	}
	if (args.verbosity >= 2) {
		printDataPoints();
	}
	if (args.verbosity >= 1) {
		printHalfCycles();
	}
	printFullCycles();
}

// destructor
Device::~Device()
{
#if(DEBUG == 1)
	cout << "Device destructor method called." << endl;
#endif
}

// print file details on screen
void Device::printFileDetails()
{
	cout << "File " << setw(3) << (file->id) << ": " << (file->name) << endl;
	cout << setw(10) << " " << "Date: " << (file->date) << endl;
	cout << setw(10) << " " << "Device: " << (file->device) << endl;
	cout << setw(10) << " " << "Plot: " << (file->plot) << endl;
	cout << setw(10) << " " << "File size: " << (file->size) << endl;
	cout << setw(10) << " " << "Records: " << (file->recs) << endl;
	cout << setw(10) << " " << "Duration (h): " << fixed << setprecision(1) <<
			( (file->testTime) / 3600.0 ) << endl;
	if ( ! (file->comment).empty() ) {
		cout << setw(10) << " " << "Comment: " << (file->comment) << endl;
	}
}

// set device type for global table
void Device::setGlobalDevice()
{
    if (details.size() > 1) {
        args.globalDevice.assign("merged");
    } else {
        args.globalDevice.assign(details[0].device);
    }
}

// process all data files
void Device::readDataFiles()
{
	for (vector<string>::iterator it = args.fileNames.begin(); it != args.fileNames.end(); ++it) {

		// create pointer
		file = new file_t({0});

		// file number, one based
		file->id = distance(args.fileNames.begin(), it) + 1;

		// file extension
        file->ext = it->substr(it->find_last_of(".") + 1);

        // test if file type is supported
        // look through keys if 'ext' exists
        if ( !(args.device.count(file->ext)) ) {
       		errorHandler(CodeFileTypeNotValid, file->ext);
    	}

		// test if file exists
		reader = new ifstream(*it, ios::binary | ios::ate);
		if ( !reader ) {
			errorHandler(CodeFileNotFound, *it);
		}

		// file size
		file->size = reader->tellg();

		// save sum of file sizes
		args.fileSizeSum += file->size;

		// file name
		file->name = *it;

		// device
		file->device = args.device.at(file->ext);

		// close file
		reader->close();

		// open file
		reader->open((*it).c_str(), ios::in);

		if ( ! (reader->is_open()) ) {
			errorHandler(CodeFileCouldNotOpen, file->name);
		}

		// Gamry file
		if (file->ext == "DTA") {
			gamryParseFileType();
			gamryParseStartTime();

			if ( (file->plot == args.plots[CodeIVCurve]) ||
					(file->plot == args.plots[CodeOpenCircuitPotential]) ) {
				gamryReadColumns();
				readDataPoints();
			}
		}

		// Arbin file
		else if (file->ext == "res") {
			arbinReadDataPoints();
		}

		// biologic text file
		else if (file->ext == "mpt") {
			biologicParseFileType();
			biologicParseStartTime();

			if ( (file->plot == args.plots[CodeIVCurve]) ||
					(file->plot == args.plots[CodeCyclicVoltammetry]) ) {
				biologicReadColumns();
				readDataPoints();
				biologicConvertCurrent();
			}
		}

		// biologic binary file
		else if (file->ext == "mpr") {
			biologicReadMprFile();
			//biologicConvertCurrent();
		}

		// ivium file
		else if (file->ext == "idf") {
			iviumParseFileType();
			iviumParseStartTime();
			iviumReadDataPoints();
		}

		// zahner
		else if (file->ext == "txt") {
			zahnerParseFileType();
			zahnerParseStartTime();

			if ( file->plot == args.plots[CodeIVCurve] ) {
				zahnerParseStartTime();
				zahnerReadColumns();
				readDataPoints();
			}
		}

		// close file and delete pointer
		reader->close();
		delete reader;

		// output file details on screen
		printFileDetails();

		// free file pointer
		details.push_back(*file);
		delete file;
	}
}

// read data points, need file pointer of open file
void Device::readDataPoints()
{
	string line;
	vector<string> tokens;
	stringstream stream;
	rec_t* record;
	size_t n = 0;

	// do not rewind, file pointer is already at data

	while ( ! reader->eof() ) {
		record = new rec_t({0});
		getline( *reader, line );
		tokens.clear();
		util::tokenize(line, tokens, "\t ");
		int signedint;

		// skip empty lines
		if (tokens.size() > 0) {

			for (size_t i = 0; i < tokens.size(); ++i) {
				signedint = static_cast<int> (i);
				if (signedint == columns[0].index) {
					// test time + time offset
					record->testTime = util::stringTo<double> (tokens[i]) + args.testTimeSum;
					// test time + start time
					record->secSinceEpoch = util::stringTo<time_t> (tokens[i]) + file->secs;
				} else if (signedint == columns[1].index) {
					// current
					record->current = util::stringTo<double> (tokens[i]);
				} else if (signedint == columns[2].index) {
					// voltage, working electrode
					record->voltage = util::stringTo<double> (tokens[i]);
				} else if (signedint == columns[3].index) {
					// voltage, counter electrode
					record->voltage2 = util::stringTo<double> (tokens[i]);
				} else if (signedint == columns[4].index) {
					// auxiliary channel (temperature)
					record->auxiliary = util::stringTo<double> (tokens[i]);
				}
			}

			record->dataPoint = n + args.recordsSum; // data point
			record->fileId = file->id; // file id
			record->localTime = ctime(&(record->secSinceEpoch)); // local time stamp in ctime format
			(record->localTime).erase((record->localTime).find_last_not_of("\n")+1); // chop newline

			// get step index
			if (record->current > 1e-15) {
				// charge
				record->stepIndex = 1;
			} else if (record->current < -1e-15) {
				// discharge
				record->stepIndex = -1;
			} else {
				// rest
				record->stepIndex = 0;
			}

			// save record
			recs.push_back(*record);
			n++;

		}

		// free pointer
		delete record;

	}

	// save global data
	(file->recs) = n; // number data points in this file
	(file->testTime) = (n == 0) ? 0 : recs.back().testTime - args.testTimeSum; // test time in this file
	args.recordsSum += n; // total data points
	args.testTimeSum += (file->testTime); // total test time
}

// print data points
void Device::printDataPoints()
{
    cout << endl << "\
+---------------------------+\n\
|    *** Data points ***    |\n\
+---------------------------+\n\
" << endl;

    	cout << setw(14) << "Data Point" <<
    			setw(14) << "Test Time" <<
				setw(14) << "Voltage" <<
				setw(14) << "Voltage2" <<
				setw(14) << "Current" <<
				setw(14) << "Auxiliary" <<
				endl;

    //size_t index = args.recordsSum - file->recs;

    for (vector<rec_t>::const_iterator it = recs.begin(); it != recs.end(); ++it) {
		cout << setw(14) << it->dataPoint <<
				setw(14) << fixed << setprecision(1) << it->testTime <<
				setw(14) << setprecision(6) << it->voltage <<
				setw(14) << setprecision(6) << it->voltage2 <<
				setw(14) << scientific << setprecision(4) << it->current <<
				setw(14) << scientific << setprecision(4) << it->auxiliary <<
				endl;
	}
}

// print data points
void Device::printCalcRecords()
{
    cout << endl << "\
+----------------------------+\n\
|    *** Calculations ***    |\n\
+----------------------------+\n\
" << endl;

    	cout << setw(14) << "Point" <<
	    		setw(8) << "Step" <<
				setw(8) << "Half" <<
	    		setw(8) << "Full" <<
				setw(14) << "Step Time" <<
				setw(14) << "Capacity" <<
				setw(14) << "Energy" <<
				setw(14) << "Energy2" <<
				setw(14) << "dQdV" <<
				setw(14) << "dQdV2" <<
				endl;

    for (vector<rec_t>::const_iterator it = recs.begin(); it != recs.end(); ++it) {
		cout << setw(14) << it->dataPoint <<
				setw(8) << it->stepIndex <<
				setw(8) << it->halfCycle <<
				setw(8) << it->fullCycle <<
				setw(14) << fixed << setprecision(1) << it->stepTime <<
				setw(14) << scientific << setprecision(4) << it->capacity <<
				setw(14) << scientific << setprecision(4) << it->energy <<
				setw(14) << scientific << setprecision(4) << it->energy2 <<
				setw(14) << scientific << setprecision(4) << it->dQdV <<
				setw(14) << scientific << setprecision(4) << it->dQdV2 <<
				endl;
	}
}

// print half cycles
void Device::printHalfCycles()
{
    cout << endl << "\
+---------------------------+\n\
|    *** Half cycles ***    |\n\
+---------------------------+\n\
" << endl;

    cout << setw(14) << "Half" <<
			setw(14) << "Begin" <<
			setw(14) << "End" <<
			setw(14) << "Time" << endl;

    for (vector<half_t>::const_iterator it = halfCycles.begin(); it != halfCycles.end(); ++it) {
		cout << setw(14) << it->halfCycle <<
				setw(14) << it->begin <<
				setw(14) << it->end <<
				setw(14) << fixed << setprecision(1) << it->stepTime << endl;
	}
}

// print full cycles
void Device::printFullCycles()
{
    cout << endl << "\
+---------------------------+\n\
|    *** Full cycles ***    |\n\
+---------------------------+\n\
" << endl;

    cout << setw(14) << "Full" <<
			setw(14) << "Begin" <<
			setw(14) << "End" <<
			setw(28) << "WE energy [Ws]" <<
			setw(28) << "CE energy [Ws]" <<
			endl;

    for (vector<full_t>::const_iterator it = fullCycles.begin(); it != fullCycles.end(); ++it) {
		cout << setw(14) << it->fullCycle <<
				setw(14) << it->begin <<
				setw(14) << it->end <<
				setw(14) << scientific << setprecision(4) << it->chargeEnergy <<
				setw(14) << scientific << setprecision(4) << it->dischargeEnergy <<
				setw(14) << scientific << setprecision(4) << it->chargeEnergy2 <<
				setw(14) << scientific << setprecision(4) << it->dischargeEnergy2 <<
				endl;
				
	}

    cout << endl;
}

// get file details
const vector<file_t>& Device::getFileDetails() const
{
	return details;
}

// get data points
const vector<rec_t>& Device::getDataPoints() const
{
	return recs;
}

// get half cycles
const vector<half_t>& Device::getHalfCycles() const
{
    return halfCycles;
}

// get full cycles
const vector<full_t>& Device::getFullCycles() const
{
    return fullCycles;
}
