#include <iostream>
#include <fstream>

#include "main.h"
#include "util.hpp"
#include "device.hpp"

using namespace std;

// get file type, ivcure, eis, etc.
void Device::gamryParseFileType()
{
	string line;
	vector<string> tokens;

	// check if gamry file
	getline( *reader, line );
	if ( line.find("EXPLAIN", 0) == string::npos ) {
		errorHandler(CodeFileNotValid, file->name);
	}

	getline( *reader, line );

	if ( (line.find("CORPOT", 0) != string::npos) ||
			(line.find("PWR800_READVOLTAGE", 0) != string::npos) ){
		file->plot = args.plots[CodeOpenCircuitPotential];

	} else if ( line.find("EIS", 0) != string::npos ) {
		file->plot = args.plots[CodeImpedanceSpectrum];

	} else if ( (line.find("PWR800_CHARGE", 0) != string::npos) ||
			(line.find("PWR800_DISCHARGE", 0) != string::npos) ||
			(line.find("PWR800_POTENTIOSTATIC", 0) != string::npos) ||
			(line.find("PWR800_GALVANOSTATIC", 0) != string::npos) ) {
		file->plot = args.plots[CodeIVCurve];

	} else {
		errorHandler(CodeDataUnknown, file->device);
	}
}

// get start time of data
void Device::gamryParseStartTime()
{
	string line;
	vector<string> tokens;
	stringstream stream;
	struct tm tm = {0};

	reader -> seekg(0); // rewind

	while ( ! reader -> eof() ) {
		getline( *reader, line );

		// parse date string
		if ( line.find("DATE", 0) != string::npos ) {
			util::tokenize(line, tokens, "\t/");
			tm.tm_mday = util::stringTo<time_t> (tokens.at(2));
			tm.tm_mon = util::stringTo<time_t> (tokens.at(3));
			tm.tm_year = util::stringTo<time_t> (tokens.at(4));
		}

		// parse time string
		if ( line.find("TIME", 0) != string::npos ) {
			tokens.clear();
        	util::tokenize(line, tokens, "\t:");
            tm.tm_hour = util::stringTo<time_t> (tokens.at(2));
            tm.tm_min = util::stringTo<time_t> (tokens.at(3));
            tm.tm_sec = util::stringTo<time_t> (tokens.at(4));
			break;
        }
	}

	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	tm.tm_isdst = -1; // ignore dst

	// save creation Date
	file -> secs = mktime(&tm);
	file -> date = asctime(&tm);
	(file -> date).erase((file -> date).find_last_not_of("\n")+1);
}

void Device::gamryReadColumns()
{
	string line;
	vector<string> tokens;
	stringstream stream;
	struct col_t col;

	columns.clear();
	col.index = -1;
	col.name = "T"; // test time
	columns.push_back(col);
	col.name = "Im"; // current
	columns.push_back(col);
	col.name = "Vf"; // voltage, working electrode
	columns.push_back(col);
	col.name = "Vu"; // voltage, reference electrode
	columns.push_back(col);
	col.name = "Temp"; // aux channel
	columns.push_back(col);

	reader->seekg(0); // rewind

	while ( ! reader->eof() ) {
		getline( *reader, line );

		// parse columns
		if ( line.find("CURVE", 0) != string::npos ) {
			getline( *reader, line );
			util::tokenize(line, tokens, "\t");
			for (size_t i = 0; i < tokens.size(); ++i) {
				if (tokens[i] == columns[0].name) {
					columns[0].index = i;
				} else if (tokens[i] == columns[1].name) {
					columns[1].index = i;
				} else if (tokens[i] == columns[2].name) {
					columns[2].index = i;
				} else if (tokens[i] == columns[3].name) {
					columns[4].index = i;
				} else if (tokens[i] == columns[4].name) {
					columns[4].index = i;
				}
			}
			break;
		}
	}

	getline( *reader, line ); // skip line with units

#if(DEBUG == 1)
	vector<col_t>::const_iterator it = columns.begin();
	cout << "Columns: ";
	while (it != columns.end()) {
		cout << (it->name) <<  ": " << (it->index) << ", ";
		it++;
	}
	cout << endl;
#endif
}
