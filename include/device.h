#ifndef DEVICE_H
#define DEVICE_H

#include "setup.h"

#if defined(_WIN32)
	#include <windows.h>
#endif
#include <string>
#include <vector>
#include <ctime>
#include <sqlext.h>

// file attributes
struct file_t {
	int id;					// current file number, one
    std::string name;		// file name
   	std::string ext;		// file extension
	std::string device;		// device type
	std::string plot;		// plot type
    size_t size;			// file size
    time_t secs;            // sec since epoch
    std::string date;		// local time string
    std::string comment;	// comment in file header
    size_t recs;			// data records per file
    double testTime;		// test time in secs
};

// data record
struct rec_t {
    int fileId;				// file ID
    size_t dataPoint;		// zero based data point
    int halfCycle;			// zero based half cycle index
    int fullCycle;			// zero based full cycle index
    int stepIndex;			// step index, 0: rest, 1: charge, -1 discharge
    double testTime;		// test time
    double stepTime;		// step time
    std::string localTime;	// local time string
    time_t secSinceEpoch;	// data point, sec since epoch
    double current;			// current
    double capacity;		// capacity
    double auxiliary;		// auxiliary channel, e.g. temperature
    double voltage;			// potential of working electrode (WE)
    double voltage2;		// potential of counter electrode (CE)
    double energy;			// energy WE
    double energy2;			// energy CE
    double dQdV;			// dQdV WE
    double dQdV2;			// dQdV CE
    
    // overload less functor for sort
    bool operator < (const rec_t& a) const {
    	return dataPoint < a.dataPoint;
    }
};

// half cycles
struct half_t {
    int halfCycle;			// zero based half cycle index
	int stepIndex;			// step index, 0: rest, 1: charge, -1 discharge
    size_t begin;			// data point, zero based cycle start
    size_t end;				// data point, zero based cycle end
    double stepTime;		// step time
	double averageCurrent;  // average current
    double capacity;		// capacity
    double energy;			// energy of working electrode
    double energy2;			// energy of counter electrode
    double averageVoltage;	// average voltage of working electrode
    double averageVoltage2;	// average voltage of counter electrode
};

// full cycles
struct full_t {
    int fullCycle;				// zero based full cycle index
    size_t begin;				// zero based cycle start
    size_t end;					// zero based cycle end
    double chargeTime;			// charge time
    double dischargeTime;		// discharge time
	double chargeCurrent;		// charge current
	double dischargeCurrent;	// discharge current
    double chargeCapacity;		// charge capacity
    double dischargeCapacity;	// discharge capacity
    double efficiency;			// coulombic efficiency
    double chargeVoltage;		// average charge voltage WE
    double dischargeVoltage;	// average discharge voltage WE
    double chargeVoltage2;		// average charge voltage CE
    double dischargeVoltage2;	// average discharge voltage CE
    double chargeEnergy;		// charge energy WE
    double dischargeEnergy;		// discharge energy WE
    double chargeEnergy2;		// charge energy CE
    double dischargeEnergy2;	// discharge energy CE
    double hysteresis;			// voltage hysteresis WE
    double hysteresis2;			// voltage hysteresis CE
};

struct col_t {
	int index;
	std::string name;
};

// instrument classes
class Device : public Setup
{
	private:
		std::ifstream* reader;
		file_t* file;
		std::vector<col_t> columns;

		// read
		void readDataFiles();
		void readDataPoints();

		// calculations
		void calculateCycles();

		// print on screen
		void printFileDetails();
		void printDataPoints();
		void printHalfCycles();
		void printCalcRecords();
		void printFullCycles();

		// methods for Gamry
		void gamryParseFileType();
		void gamryParseStartTime();
		void gamryReadColumns();

		// methods for Arbin
		void showError(SQLSMALLINT, const SQLHANDLE&);
		void arbinReadDataPoints();

		// methods for Biologic
		void biologicParseFileType();
		void biologicParseStartTime();
		void biologicReadColumns();
		void biologicConvertCurrent();
		void biologicReadMprFile();

		// methods for Ivium
		void iviumParseFileType();
		void iviumParseStartTime();
		void iviumReadDataPoints();

		// methods for Zahner
		void zahnerParseFileType();
		void zahnerParseStartTime();
		void zahnerReadColumns();

	protected:
		std::vector<file_t> details;
		std::vector<rec_t> recs;
		std::vector<half_t> halfCycles;
		std::vector<full_t> fullCycles;

	public:
		Device(int, char**);
		~Device();

		// get methods
		const std::vector<file_t>& getFileDetails() const;
		const std::vector<rec_t>& getDataPoints() const;
		const std::vector<half_t>& getHalfCycles() const;
		const std::vector<full_t>& getFullCycles() const;
};

#endif // DEVICE_H
