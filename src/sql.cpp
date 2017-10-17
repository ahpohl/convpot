#include "main.h"
#include "util.h"
#include "msql.h"

#include <iostream>
#include <ctime>
using namespace std;

// sql constructor
Sql::Sql(int argc, char** argv) : Device(argc, argv)
{
#if(DEBUG == 1)
    cout << "Sql constructor method called." << endl;
#endif

	// save
	writeSqlite();
}

// destructor
Sql::~Sql()
{
#if(DEBUG == 1)
	cout << "Sql destructor method called." << endl;
#endif
}

void Sql::execQuery(string query)
{
	if (sqlite3_exec(db, query.c_str(), NULL, NULL, &errMsg) != SQLITE_OK) {
		errorHandler(CodeSqliteError, errMsg);
	}
}

void Sql::execPrepare(string query)
{
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, &tail) != SQLITE_OK) {
		errorHandler(CodeSqliteError, "sqlite3_prepare_v2 failed");
	}
}

void Sql::writeGlobalTable()
{
	// drop table if exists
	sqlQuery = "DROP TABLE IF EXISTS Global_Table";
	execQuery(sqlQuery);

	// create table
	sqlQuery = "CREATE TABLE Global_Table (\
File_Name TEXT,\
File_Size INTEGER,\
DateTime INTEGER,\
Data_Points INTEGER,\
Device TEXT,\
Mass DOUBLE,\
Capacity DOUBLE,\
Area DOUBLE,\
Volume DOUBLE,\
Version TEXT)";
	execQuery(sqlQuery);

	// insert values
	sqlQuery = "INSERT INTO Global_Table VALUES ('" +
			args.outputFilename + "'," +
			util::toString(args.fileSizeSum) + "," +
			util::toString(time(0)) + "," +
			util::toString(args.recordsSum) + ",'" +
			args.globalDevice +
			"',0,0,0,0,'" + 
			args.fullVersion + "')";
	execQuery(sqlQuery);
}

void Sql::writeFileTable()
{
	// drop table if exists
	sqlQuery = "DROP TABLE IF EXISTS File_Table";
	execQuery(sqlQuery);

	// create table
	sqlQuery = "CREATE TABLE File_Table (\
File_ID INTEGER PRIMARY KEY,\
File_Name TEXT,\
Device TEXT,\
Plot_Type TEXT,\
File_Size INTEGER,\
Start_DateTime INTEGER,\
Localtime TEXT,\
Data_Points INTEGER,\
Test_Time DOUBLE,\
Comment TEXT)";
	execQuery(sqlQuery);

	// bind records
	sqlQuery = "INSERT INTO File_Table VALUES (?,?,?,?,?,?,?,?,?,?)";
	execPrepare(sqlQuery);
	execQuery("BEGIN TRANSACTION");

	// insert values
	for (vector<file_t>::const_iterator it = details.begin(); it != details.end(); ++it) {
		sqlite3_bind_int(stmt, 1, it->id);
		sqlite3_bind_text(stmt, 2, it->name.c_str(), it->name.length(), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, it->device.c_str(), it->device.length(), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 4, it->plot.c_str(), it->plot.length(), SQLITE_TRANSIENT);
		sqlite3_bind_int64(stmt, 5, it->size);
		sqlite3_bind_int64(stmt, 6, it->secs);
		sqlite3_bind_text(stmt, 7, it->date.c_str(), it->date.length(), SQLITE_TRANSIENT);
		sqlite3_bind_int64(stmt, 8, it->recs);
		sqlite3_bind_double(stmt, 9, it->testTime);
		sqlite3_bind_text(stmt, 10, it->comment.c_str(), it->comment.length(), SQLITE_TRANSIENT);

		sqlite3_step(stmt); // Execute the SQL Statement
		sqlite3_clear_bindings(stmt); // Clear bindings
		sqlite3_reset(stmt); // Reset VDBE
	}

	execQuery("END TRANSACTION");
	sqlite3_finalize(stmt);
}

void Sql::writeNormalTable()
{
	// drop table if exists
	sqlQuery = "DROP TABLE IF EXISTS Channel_Normal_Table";
	execQuery(sqlQuery);

	// create table
	sqlQuery = "CREATE TABLE Channel_Normal_Table (\
Test_ID INTEGER,\
Data_Point INTEGER PRIMARY KEY,\
Half_Cycle INTEGER,\
Full_Cycle INTEGER,\
Step_Index INTEGER,\
Test_Time DOUBLE,\
Step_Time DOUBLE,\
Localtime TEXT,\
DateTime DOUBLE,\
Aux_Channel DOUBLE,\
Current DOUBLE,\
Capacity DOUBLE,\
Voltage DOUBLE,\
Voltage2 DOUBLE,\
Energy DOUBLE,\
Energy2 DOUBLE,\
dQdV DOUBLE,\
dQdV2 DOUBLE)";
	execQuery(sqlQuery);

	// bind records
	sqlQuery = "INSERT INTO Channel_Normal_Table VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
	execPrepare(sqlQuery);
	execQuery("BEGIN TRANSACTION");

	// insert values
	for (vector<rec_t>::const_iterator it = recs.begin(); it != recs.end(); ++it) {
		sqlite3_bind_int(stmt, 1, it->fileId);
		sqlite3_bind_int64(stmt, 2, it->dataPoint);
		sqlite3_bind_int(stmt, 3, it->halfCycle);
		sqlite3_bind_int(stmt, 4, it->fullCycle);
		sqlite3_bind_int(stmt, 5, it->stepIndex);
		sqlite3_bind_double(stmt, 6, it->testTime);
		sqlite3_bind_double(stmt, 7, it->stepTime);
		sqlite3_bind_text(stmt, 8, it->localTime.c_str(), it->localTime.length(), SQLITE_TRANSIENT);
		sqlite3_bind_double(stmt, 9, it->secSinceEpoch);
		sqlite3_bind_double(stmt, 10, it->auxiliary);
		sqlite3_bind_double(stmt, 11, it->current);
		sqlite3_bind_double(stmt, 12, it->capacity);
		sqlite3_bind_double(stmt, 13, it->voltage);
		sqlite3_bind_double(stmt, 14, it->voltage2);
		sqlite3_bind_double(stmt, 15, it->energy);
		sqlite3_bind_double(stmt, 16, it->energy2);
		sqlite3_bind_double(stmt, 17, it->dQdV);
		sqlite3_bind_double(stmt, 18, it->dQdV2);

		sqlite3_step(stmt); // Execute the SQL Statement
		sqlite3_clear_bindings(stmt); // Clear bindings
		sqlite3_reset(stmt); // Reset VDBE
	}

	execQuery("END TRANSACTION");
	sqlite3_finalize(stmt);
}

void Sql::writeHalfCycleTable()
{
	// drop table if exists
	sqlQuery = "DROP TABLE IF EXISTS Half_Cycle_Table";
	execQuery(sqlQuery);

	// create table
	sqlQuery = "CREATE TABLE Half_Cycle_Table (\
Half_Cycle INTEGER PRIMARY KEY,\
Step_Index INTEGER,\
Cycle_Start INTEGER,\
Cycle_End INTEGER,\
Step_time DOUBLE,\
Average_Current DOUBLE,\
Capacity DOUBLE,\
Energy DOUBLE,\
Energy2 DOUBLE,\
Average_Voltage DOUBLE,\
Average_Voltage2 DOUBLE)";
	execQuery(sqlQuery);

	// bind records
	sqlQuery = "INSERT INTO Half_Cycle_Table VALUES (?,?,?,?,?,?,?,?,?,?,?)";
	execPrepare(sqlQuery);
	execQuery("BEGIN TRANSACTION");

	// insert values
	for (vector<half_t>::const_iterator it = halfCycles.begin(); it != halfCycles.end(); ++it) {
		sqlite3_bind_int(stmt, 1, it->halfCycle);
		sqlite3_bind_int(stmt, 2, it->stepIndex);
		sqlite3_bind_int64(stmt, 3, it->begin);
		sqlite3_bind_int64(stmt, 4, it->end);
		sqlite3_bind_double(stmt, 5, it->stepTime);
		sqlite3_bind_double(stmt, 6, it->averageCurrent);
		sqlite3_bind_double(stmt, 7, it->capacity);
		sqlite3_bind_double(stmt, 8, it->energy);
		sqlite3_bind_double(stmt, 9, it->energy2);
		sqlite3_bind_double(stmt, 10, it->averageVoltage);
		sqlite3_bind_double(stmt, 11, it->averageVoltage2);

		sqlite3_step(stmt); // Execute the SQL Statement
		sqlite3_clear_bindings(stmt); // Clear bindings
		sqlite3_reset(stmt); // Reset VDBE
	}

	execQuery("END TRANSACTION");
	sqlite3_finalize(stmt);
}

void Sql::writeFullCycleTable()
{
	// drop table if exists
	sqlQuery = "DROP TABLE IF EXISTS Full_Cycle_Table";
	execQuery(sqlQuery);

	// create table
	sqlQuery = "CREATE TABLE Full_Cycle_Table (\
Full_Cycle INTEGER PRIMARY KEY,\
Cycle_Start INTEGER,\
Cycle_End INTEGER,\
Charge_Time DOUBLE,\
Discharge_Time DOUBLE,\
Charge_Current DOUBLE,\
Discharge_Current DOUBLE,\
Charge_Capacity DOUBLE,\
Discharge_Capacity DOUBLE,\
Efficiency DOUBLE,\
Charge_Voltage DOUBLE,\
Discharge_Voltage DOUBLE,\
Charge_Voltage2 DOUBLE,\
Discharge_Voltage2 DOUBLE,\
Charge_Energy DOUBLE,\
Discharge_Energy DOUBLE,\
Charge_Energy2 DOUBLE,\
Discharge_Energy2 DOUBLE,\
Hysteresis DOUBLE,\
Hysteresis2 DOUBLE)";
	execQuery(sqlQuery);

	// bind records
	sqlQuery = "INSERT INTO Full_Cycle_Table VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
	execPrepare(sqlQuery);
	execQuery("BEGIN TRANSACTION");

	// insert values
	for (vector<full_t>::const_iterator it = fullCycles.begin(); it != fullCycles.end(); ++it) {
		sqlite3_bind_int(stmt, 1, it->fullCycle);
		sqlite3_bind_int64(stmt, 2, it->begin);
		sqlite3_bind_int64(stmt, 3, it->end);
		sqlite3_bind_double(stmt, 4, it->chargeTime);
		sqlite3_bind_double(stmt, 5, it->dischargeTime);
		sqlite3_bind_double(stmt, 6, it->chargeCurrent);
		sqlite3_bind_double(stmt, 7, it->dischargeCurrent);
		sqlite3_bind_double(stmt, 8, it->chargeCapacity);
		sqlite3_bind_double(stmt, 9, it->dischargeCapacity);
		sqlite3_bind_double(stmt, 10, it->efficiency);
		sqlite3_bind_double(stmt, 11, it->chargeVoltage);
		sqlite3_bind_double(stmt, 12, it->dischargeVoltage);
		sqlite3_bind_double(stmt, 13, it->chargeVoltage2);
		sqlite3_bind_double(stmt, 14, it->dischargeVoltage2);
		sqlite3_bind_double(stmt, 15, it->chargeEnergy);
		sqlite3_bind_double(stmt, 16, it->dischargeEnergy);
		sqlite3_bind_double(stmt, 17, it->chargeEnergy2);
		sqlite3_bind_double(stmt, 18, it->dischargeEnergy2);
		sqlite3_bind_double(stmt, 19, it->hysteresis);
		sqlite3_bind_double(stmt, 20, it->hysteresis2);

		sqlite3_step(stmt); // Execute the SQL Statement
		sqlite3_clear_bindings(stmt); // Clear bindings
		sqlite3_reset(stmt); // Reset VDBE
	}

	execQuery("END TRANSACTION");
	sqlite3_finalize(stmt);
}

void Sql::writeSqlite()
{
	// open sqlite file
	sqlite3_open(args.outputFilename.c_str(), &db);

	// insert tables
	writeGlobalTable();
	writeFileTable();
	writeNormalTable();
	writeHalfCycleTable();
	writeFullCycleTable();

	// close db
	sqlite3_close(db);

	// screen output
	cout << "Output filename: " << args.outputFilename << endl;
}
