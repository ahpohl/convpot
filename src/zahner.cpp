#include "main.h"
#include "util.h"
#include "device.h"

#include <iostream>
#include <fstream>
using namespace std;

// get file type, ivcure, eis, etc.
void Device::zahnerParseFileType()
{
	string line;
	vector<string> tokens;

	// check if zahner file
	while ( ! reader -> eof() ) {
		getline( *reader, line );
		if ( line.find("Time/sec     Potential/V", 0) != string::npos ) {
			file -> plot = args.plots[CodeIVCurve];
			break;
		} else if ( line.find("Impedance samples", 0) != string::npos ) {
			file -> plot = args.plots[CodeImpedanceSpectrum];
			break;
		} else if ( line.find("Cyclic voltammogram", 0) != string::npos ) {
			file -> plot = args.plots[CodeCyclicVoltammetry];
			break;
		}
	}

	if (reader -> eof()) {
		errorHandler(CodeDataUnknown, file -> device);
	}
}

// get start time of data
void Device::zahnerParseStartTime()
{
	// zahner has no creation time, use current system time
	file -> secs = time(NULL);
	file -> date = ctime(&(file->secs));
	(file -> date).erase((file -> date).find_last_not_of("\n")+1);

	// TODO: ask user for creation time, needed for in situ raman
}

void Device::zahnerReadColumns()
{
	string line;
	vector<string> tokens;
	stringstream stream;
	struct col_t col;

	columns.clear();
	col.index = -1;
	col.name = "Time/sec"; // test time
	columns.push_back(col);
	col.name = "Current/A"; // current
	columns.push_back(col);
	col.name = "Potential/V"; // voltage, working electrode
	columns.push_back(col);
	col.name = "not exist"; // voltage, reference electrode, dummy
	columns.push_back(col);
	col.name = "not exist"; // aux channel
	columns.push_back(col);

	reader->seekg(0); // rewind

	getline( *reader, line );

	// parse columns
	util::tokenize(line, tokens, " ");
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == columns[0].name) {
			columns[0].index = i;
		} else if (tokens[i] == columns[1].name) {
			columns[1].index = i;
		} else if (tokens[i] == columns[2].name) {
			columns[2].index = i;
		}
	}

	getline( *reader, line ); // skip empty line

#if(DEBUG == 1)
	vector<col_t>::iterator it = columns.begin();
	cout << "Columns: ";
	while (it != columns.end()) {
		cout << (it->name) <<  ": " << (it->index) << ", ";
		it++;
	}
	cout << endl;
#endif
}
