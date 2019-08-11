#include <iostream>
#include <algorithm>

#include "main.h"
#include "device.hpp"
#include "math.h"

using namespace std;

// error handler for odbc sql queries
void Device::showError(SQLSMALLINT handleType, const SQLHANDLE& handle)
{
	SQLCHAR sqlState[1024];
	SQLCHAR messageText[1024];
	SQLRETURN retCode;
	SQLINTEGER i = 0;
	SQLSMALLINT	len;
	SQLINTEGER native;
	do {
		retCode = SQLGetDiagRec(handleType, handle, ++i, sqlState, &native, messageText, 1024, &len);
		if (SQL_SUCCEEDED(retCode)) {
			cout << "Message " << i << ": " << messageText << endl <<
					"SQLSTATE: " << sqlState <<
					"; Native: " << native << endl;
		}
	} while (retCode == SQL_SUCCESS);
}

// read data using odbc
void Device::arbinReadDataPoints() {

	// odbc handles
	SQLHANDLE sqlEnvHandle;
	SQLHANDLE sqlConnectionHandle;
	SQLHANDLE sqlStatementHandle;

	// connection string and query
	string inConString;
	SQLCHAR retConString[255];
	string sqlQuery;
	SQLRETURN retCode;
	SQLSMALLINT connStrLength;
	SQLLEN ret;
	SQLCHAR driver[256];
	SQLCHAR attr[256];
	SQLSMALLINT retDriver;

	// data
	rec_t* record = NULL;
	char comments[1024] = {0};
	double startTime = 0;
	size_t dataPoint = 0;
	double testTime = 0, voltage = 0, current = 0, auxChannel = 0;
	size_t row = 0, x = 0;

	// create a handle for the environment
	retCode = SQLAllocEnv(&sqlEnvHandle);

	// allocate the connection handle.
	retCode = SQLAllocConnect(sqlEnvHandle, &sqlConnectionHandle);

#if defined(__linux)
	inConString = "Driver=MDBToolsODBC;DBQ=" + file->name + ";UID=;PWD=;";

#elif defined(_WIN32)
	char cPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cPath);
	string fullPath(cPath);
	fullPath.append("\\"+file->name);

	// get ODBC data sources
	while(SQL_SUCCEEDED(retCode = SQLDrivers(sqlEnvHandle,
			SQL_FETCH_NEXT,
			driver, sizeof(driver), &retDriver,
			attr, sizeof(attr), &retDriver))) {

		if ( strcmp((char*)driver, "Microsoft Access Driver (*.mdb, *.accdb)") == 0 ) {
			inConString = "Driver={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=" + fullPath + ";UID=;PWD=;";
			break;
		}
	}

	if (inConString.empty()) {
		inConString = "Driver={Microsoft Access Driver (*.mdb)};DBQ=" + fullPath + ";UID=;PWD=;";
	}
#endif

#if(DEBUG == 1)
	cout << "inConString: " << inConString << endl;
#endif

	// connect to the database.
    retCode = SQLDriverConnect (sqlConnectionHandle,
    		NULL, (SQLCHAR*)inConString.c_str(), SQL_NTS,
			retConString, 255, &connStrLength, SQL_DRIVER_NOPROMPT);

	if (!SQL_SUCCEEDED(retCode)) {
		// show error
		cout << "Connection string: " << retConString << endl;
		showError(SQL_HANDLE_DBC, sqlConnectionHandle);

		// get ODBC data sources
    	while(SQL_SUCCEEDED(retCode = SQLDrivers(sqlEnvHandle, SQL_FETCH_NEXT,
    			driver, sizeof(driver), &retDriver,
    			attr, sizeof(attr), &retDriver))) {
    		cout << driver << " - " << attr << endl;
    	}

		// exit program
		errorHandler(CodeODBCerror);
	}

	// Create and allocate a statement
	retCode = SQLAllocStmt(sqlConnectionHandle, &sqlStatementHandle);

	// select comment and start time from global table
	sqlQuery = "SELECT Comments,Start_DateTime FROM Global_Table";

	retCode = SQLExecDirect(sqlStatementHandle, (SQLCHAR*) sqlQuery.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(retCode)) {
		showError(SQL_HANDLE_STMT, sqlStatementHandle);
		errorHandler(CodeODBCerror);
	}

	// retrieve first row
	retCode = SQLFetch(sqlStatementHandle);
	if (!SQL_SUCCEEDED(retCode)) {
		showError(SQL_HANDLE_STMT, sqlStatementHandle);
		errorHandler(CodeODBCerror);
	}
	retCode = SQLGetData(sqlStatementHandle, 1, SQL_C_CHAR, (SQLCHAR*) comments, 1024, &ret);
	retCode = SQLGetData(sqlStatementHandle, 2, SQL_C_DOUBLE, (SQLDOUBLE*) &startTime, 0, &ret);

	// process start time and comment
	// convert from Excel date to secs since epoch
	(file->comment).assign(comments);
	file->secs = (time_t) round((startTime - 25569) * 86400);
	file->date = ctime(&(file->secs));
	(file->date).erase((file->date).find_last_not_of("\n")+1);

	// default ivcurve, no keywords to check
	file->plot = args.plots[CodeIVCurve];

	// finish query
	retCode = SQLCloseCursor(sqlStatementHandle);

	// retrieve records
    sqlQuery = "SELECT Data_Point,Test_Time,Current,Voltage FROM Channel_Normal_Table";

    // prepare query
    retCode = SQLPrepare(sqlStatementHandle, (SQLCHAR*) sqlQuery.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(retCode)) {
		showError(SQL_HANDLE_STMT, sqlStatementHandle);
		errorHandler(CodeODBCerror);
	}

#if defined(__linux)
	SQLCHAR buf1[256], buf2[256], buf3[256], buf4[256];
	// Bind columns 1, 2, 3 and 4
	retCode = SQLBindCol(sqlStatementHandle, 1, SQL_C_CHAR, buf1, 256, &ret);
	retCode = SQLBindCol(sqlStatementHandle, 2, SQL_C_CHAR, buf2, 256, &ret);
	retCode = SQLBindCol(sqlStatementHandle, 3, SQL_C_CHAR, buf3, 256, &ret);
	retCode = SQLBindCol(sqlStatementHandle, 4, SQL_C_CHAR, buf4, 256, &ret);

#elif defined(_WIN32)
	// Bind columns 1, 2, 3 and 4
	retCode = SQLBindCol(sqlStatementHandle, 1, SQL_C_LONG, (SQLLEN*) &dataPoint, 0, &ret);
	retCode = SQLBindCol(sqlStatementHandle, 2, SQL_C_DOUBLE, (SQLDOUBLE*) &testTime, 0, &ret);
	retCode = SQLBindCol(sqlStatementHandle, 3, SQL_C_DOUBLE, (SQLDOUBLE*) &current, 0, &ret);
	retCode = SQLBindCol(sqlStatementHandle, 4, SQL_C_DOUBLE, (SQLDOUBLE*) &voltage, 0, &ret);
#endif

	// execute query
	retCode = SQLExecute(sqlStatementHandle);
	if (!SQL_SUCCEEDED(retCode)) {
		showError(SQL_HANDLE_STMT, sqlStatementHandle);
		errorHandler(CodeODBCerror);
	}

	// Fetch the data
	while (SQL_SUCCEEDED(retCode = SQLFetch(sqlStatementHandle))) {
		record = new rec_t({0});

#if defined(__linux)
		// data type conversion
		dataPoint = strtol((char*)buf1, NULL, 10);
		testTime = strtod((char*)buf2, NULL);
		current = strtod((char*)buf3, NULL);
		voltage = strtod((char*)buf4, NULL);
#endif
		record->dataPoint = dataPoint + args.recordsSum - 1; // arbin: 1 based data point, want 0 based
		record->testTime = testTime + args.testTimeSum;
		record->current = current;
		record->voltage = voltage;

		record->secSinceEpoch = (time_t) round(testTime) + file->secs;
		record->localTime = ctime(&(record->secSinceEpoch));
		(record->localTime).erase((record->localTime).find_last_not_of("\n")+1);
		record->fileId = file->id;

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

		recs.push_back(*record);
		delete record;
		row++;
	}

	// finish query
	retCode = SQLCloseCursor(sqlStatementHandle);

	// retrieve aux channel data
    sqlQuery = "SELECT X FROM Auxiliary_Table";

    // execute query
	retCode = SQLExecDirect(sqlStatementHandle, (SQLCHAR*) sqlQuery.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(retCode)) {
		showError(SQL_HANDLE_STMT, sqlStatementHandle);
	}

	// Fetch the data
	while (SQL_SUCCEEDED(retCode = SQLFetch(sqlStatementHandle))) {
#if defined(__linux)
		SQLGetData(sqlStatementHandle, 1, SQL_C_CHAR, buf1, 256, &ret);
		auxChannel = strtod((char*)buf1, NULL);
#elif defined(_WIN32)
		SQLGetData(sqlStatementHandle, 1, SQL_C_DOUBLE, (SQLDOUBLE*) &auxChannel, 0, &ret);
#endif
		recs[(x++)+args.recordsSum].auxiliary = auxChannel;
	}

	// finish query
	retCode = SQLCloseCursor(sqlStatementHandle);

	// save global data
	(file->recs) = row; // number data points in this file
	(file->testTime) = (row == 0) ? 0 : recs.back().testTime - args.testTimeSum; // test time in this file
	args.recordsSum += row; // total data points
	args.testTimeSum += (file->testTime); // total test time

	// sort recs by data point using default comparison (operator <):
	sort((recs.end() - file->recs), recs.end());

	// free up resources
	SQLFreeHandle(SQL_HANDLE_STMT, sqlStatementHandle);
	SQLDisconnect(sqlConnectionHandle);
	SQLFreeHandle(SQL_HANDLE_DBC, sqlConnectionHandle);
	SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
}
