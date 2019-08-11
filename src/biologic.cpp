#include <iostream>
#include <fstream>

#include "main.h"
#include "util.hpp"
#include "device.hpp"

using namespace std;

// get file type, ivcure, eis, etc.
void Device::biologicParseFileType()
{
	string line;
	vector<string> tokens;

	// check if biologic file
	getline( *reader, line );
	if ( line.find("EC-Lab ASCII FILE", 0) == string::npos ) {
		errorHandler(CodeFileNotValid, file -> name);
	}

	for (int i = 0; i < 3; ++i) {
		getline( *reader, line );
	}

	if ( line.find("Impedance Spectroscopy", 0) != string::npos ) {
		file -> plot = args.plots[CodeImpedanceSpectrum];

	} else if ( line.find("Galvanostatic Cycling", 0) != string::npos ) {
		file -> plot = args.plots[CodeIVCurve];

	} else if ( line.find("Cyclic Voltammetry", 0) != string::npos ) {
		file -> plot = args.plots[CodeCyclicVoltammetry];

	} else {
		errorHandler(CodeDataUnknown, file -> device);
	}
}

// get start time of data
void Device::biologicParseStartTime()
{
	string line;
	vector<string> tokens;
	stringstream stream;
	struct tm tm = {0};

	reader -> seekg(0); // rewind

	while ( ! reader -> eof() ) {
		getline( *reader, line );

		// parse date and time
		if ( line.find("Acquisition started on", 0) != string::npos ) {
			util::tokenize(line, tokens, " /:");
			tm.tm_mday = util::stringTo<time_t> (tokens.at(4));
			tm.tm_mon = util::stringTo<time_t> (tokens.at(3));
			tm.tm_year = util::stringTo<time_t> (tokens.at(5));
            tm.tm_hour = util::stringTo<time_t> (tokens.at(6));
            tm.tm_min = util::stringTo<time_t> (tokens.at(7));
            tm.tm_sec = util::stringTo<time_t> (tokens.at(8));
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

void Device::biologicReadColumns()
{
	string line;
	vector<string> tokens;
	stringstream stream;
	struct col_t col;

	columns.clear();
	col.index = -1;
	col.name = "time/s"; // test time
	columns.push_back(col);
	col.name = "<I>/mA"; // current
	columns.push_back(col);
	col.name = "Ewe/V"; // voltage, working electrode
	columns.push_back(col);
	col.name = "Ece/V"; // voltage, reference electrode
	columns.push_back(col);
	col.name = "not exist"; // auxiliary channel, dummy
	columns.push_back(col);

	reader->seekg(0); // rewind

	while ( ! reader->eof() ) {
		getline( *reader, line );
		// parse columns
		if ( line.find("mode", 0) != string::npos ) {
			util::tokenize(line, tokens, "\t");
			for (size_t i = 0; i < tokens.size(); ++i) {
				if (tokens[i] == columns[0].name) {
					columns[0].index = i;
				} else if (tokens[i] == columns[1].name) {
					columns[1].index = i;
				} else if (tokens[i] == columns[2].name) {
					columns[2].index = i;
				} else if (tokens[i] == columns[3].name) {
					columns[3].index = i;
				}
			}
			break;
		}
	}

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

// convert mA to A
void Device::biologicConvertCurrent()
{
    // biologic saves current in [mA], convert to [A]
	for (vector<rec_t>::iterator it = recs.begin(); it != recs.end(); ++it) {
		it->current *= 1e-3;
	}
}

void Device::biologicReadMprFile()
{
	rec_t* record;
	size_t n = 0;
	struct tm tm = {0};

	// data address
	const int offset = 0x1B;
	const int timePos = 0xA61;
	const int currentPos = 0x9D174;
	const int voltagePos = 0x94462;
	const int datePos = 0x8C1;

	// close already opened file stream
	reader->close();

	// re-open file binary mode
	reader->open(file->name, ios::in | ios::binary);

	// read time
	while ( ! reader->eof() ) {
		record = new rec_t({0});

		// time
		reader->seekg(timePos + n * offset);
		reader->read( reinterpret_cast<char*>( &(record->testTime) ), sizeof(double) );

		// current
		reader->seekg(currentPos + n * offset);
		reader->read( reinterpret_cast<char*>( &(record->current) ), sizeof(double) );

		// voltage
		reader->seekg(voltagePos + n * offset);
		reader->read( reinterpret_cast<char*>( &(record->voltage) ), sizeof(double) );

		// save record
		recs.push_back(*record);
		n++;

		// free pointer
		delete record;

		if (n == 1000)
			break;

	}

	// date
	char *buffer = new char[9];
	vector<string> tokens;

	reader->clear();
	reader->seekg(datePos);
	reader->read( buffer, 8 );

	string dateStr( buffer, 8 );
	delete[] buffer;

	util::tokenize(dateStr, tokens, "/");
	tm.tm_mday = util::stringTo<time_t> (tokens.at(1));
	tm.tm_mon = util::stringTo<time_t> (tokens.at(0));
	tm.tm_year = util::stringTo<time_t> (tokens.at(2));

	tm.tm_year += 100;
	tm.tm_mon -= 1;
	tm.tm_isdst = -1; // ignore dst

	// save creation Date
	file -> secs = mktime(&tm);
	file -> date = asctime(&tm);
	(file -> date).erase((file -> date).find_last_not_of("\n")+1);
}
