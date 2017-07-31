#include "util.h"
#include "device.h"

#include <iostream>
#include <fstream>
using namespace std;

// get file type, ivcure, eis, etc.
void Device::iviumParseFileType()
{
	string line;
	vector<string> tokens;

	// check if ivium file, skip first line with binary data
	getline( *reader, line );
	getline( *reader, line );
	if ( line.find("QR=", 0) == string::npos ) {
		errorHandler(CodeFileNotValid, file -> name);
	}

	// default ivcurve, no keywords to check
	file -> plot = args.plots[CodeIVCurve];
}

// get start time of data
void Device::iviumParseStartTime()
{
	string line;
	vector<string> tokens;
	stringstream stream;
	struct tm tm = {0};

	reader -> seekg(0); // rewind

	while ( ! reader -> eof() ) {
		getline( *reader, line );

		// parse date and time string
		if ( line.find("starttime", 0) != string::npos ) {
			util::tokenize(line, tokens, "=/: ");
			tm.tm_mday = util::stringTo<time_t> (tokens.at(2));
			tm.tm_mon = util::stringTo<time_t> (tokens.at(1));
			tm.tm_year = util::stringTo<time_t> (tokens.at(3));
            tm.tm_hour = util::stringTo<time_t> (tokens.at(4));
            tm.tm_min = util::stringTo<time_t> (tokens.at(5));
            tm.tm_sec = util::stringTo<time_t> (tokens.at(6));
			break;
        }
	}

	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	tm.tm_isdst = -1; // ignore dst
	if (tokens.at(7) == "PM") {
		tm.tm_hour += 12;
	}

	// save creation Date
	file -> secs = mktime(&tm);
	file -> date = asctime(&tm);
	(file -> date).erase((file -> date).find_last_not_of("\n")+1);
}

void Device::iviumReadDataPoints()
{
	string line;
	vector<string> tokens;
	rec_t* record;
	size_t row = 0;

	reader->seekg(0); // rewind

	while ( ! reader->eof() ) {
		getline( *reader, line );

		// parse columns
		if ( line.find("primary_data", 0) != string::npos ) {
			for (int i = 0; i < 2; ++i) {
				getline( *reader, line );
			}
			break;
		}
	}

	while ( ! reader->eof() ) {
		record = new rec_t({0});
		getline( *reader, line );
		tokens.clear();
		util::tokenize(line, tokens, " ");

		 // skip empty lines
		if (tokens.size() > 0) {

			// test time + time offset
			record->testTime = util::stringTo<double> (tokens.at(0)) + args.testTimeSum;
			// test time + start time
			record->secSinceEpoch = util::stringTo<time_t> (tokens.at(0)) + file->secs;
			// current
			record->current = util::stringTo<double> (tokens.at(1));
			// voltage, working electrode
			record->voltage = util::stringTo<double> (tokens.at(2));

			record->dataPoint = row + args.recordsSum; // data point
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
			row++;
		}

		// free pointer
		delete record;
	}

	// save global data
	(file->recs) = row; // number data points in this file
	(file->testTime) = (row == 0) ? 0 : recs.back().testTime - args.testTimeSum; // test time in this file
	args.recordsSum += row; // total data points
	args.testTimeSum += (file->testTime); // total test time
}
