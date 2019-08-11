#ifndef SQL_HPP
#define SQL_HPP

#include "device.hpp"
#include <sqlite3.h>

class Sql : public Device
{
private:
  sqlite3* db;
  sqlite3_stmt* stmt = NULL;
  const char* tail = NULL;
  char* errMsg = NULL;
  std::string sqlQuery;

  // sql manager methods
  void execQuery(std::string);
  void execPrepare(std::string);

  // save tables
  void writeGlobalTable();
  void writeFileTable();
  void writeNormalTable();
  void writeHalfCycleTable();
  void writeFullCycleTable();

  // read tables
  void readGlobalTable();
  void readFileTable();
  void readNormalTable();
  void readHalfCycleTable();
  void readFullCycleTable();

public:
  Sql(int, char**);
	~Sql();

  void readSqlite();
  void writeSqlite();
};

#endif // SQL_HPP
