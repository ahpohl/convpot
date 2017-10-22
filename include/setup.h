#ifndef SETUP_H
#define SETUP_H

#include <vector>
#include <map>
#include <chrono>

// return codes for all classes
enum ReturnCodes
{
    CodeOk,
	CodeError,
	CodeCmdParseError,
	CodeVersionRequested,
	CodeHelpRequested,
	CodeInfoRequested,
	CodeFileNotFound,
	CodeFileCouldNotOpen,
	CodeFileTypeNotValid,
	CodeFileNotValid,
	CodeEmptyListFile,
	CodeDataUnknown,
	CodeIVCurve,
	CodeCyclicVoltammetry,
	CodeImpedanceSpectrum,
	CodeOpenCircuitPotential,
	CodeCtimeError,
	CodeODBCerror,
	CodeSqliteError
};

// option parser and global variables
struct global_t {
    bool isVersion;            		// -V option, flag
    bool isCounterElectrode;   		// -c option, flag
    int verbosity;          		// -v option, flag
    bool isTimer;					// -b option, flag
    std::string listFilename;		// -f option, list file name
	std::string outputFilename;		// -o option, output file name
    std::vector<std::string> fileNames;	// list of filenames
	std::map<std::string, std::string> device;	// map of file ext and instrument desc
	std::string globalDevice;       // device for global table
	std::map<ReturnCodes, std::string> plots;	// map of experimental data types
    size_t fileSizeSum;		       	// total file size of all input files
    size_t recordsSum;       		// total data records
    double testTimeSum;      		// total test time
    std::string fullVersion;		// full git version
    std::string shortVersion;		// short version 
    int smooth;						// smooth, integer 1-4
};

class return_exception: public std::exception
{
	public:
		inline return_exception(const char* errMsg = "Return exception."): _msg(errMsg) {};
		inline ~return_exception(void) {};
		inline const char* what() const throw() { return _msg; };

	private:
		const char* _msg;
};

// setup program, fill global structure with cmd args
class Setup
{
	private:
		int argc;
		char** argv;
		void setDevices();
		void displayHelp();
		void displayInfo();
		void displayHeader();
		void parseListFile();
		void setOutputFilename();
		void parseCmdLine();

		// benchmark
		std::chrono::steady_clock::time_point clockStart;
		void startTimer();
		void stopTimer();

	protected:
		global_t args;
		void errorHandler(ReturnCodes ret, const std::string& = "error message");

	public:
		Setup(int argc, char** argv);
		~Setup();
		global_t getArguments();
};

#endif // SETUP_H
