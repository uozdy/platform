#ifdef ENABLE_MYSQL

#ifndef __IO_MYSQL_EXT_H__
#define __IO_MYSQL_EXT_H__
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string.h>
#include <functional>

namespace PLATFORM {
	typedef std::function<std::string(sql::ResultSet* res)> QUERYRESULTSETFN;
	typedef std::function<void(sql::PreparedStatement* stmt)> UPDATESTMTFN;
	typedef std::function<bool(sql::SQLException& err)> ERRORFN;

	typedef struct {
		const char* host;
		const char* user;
		const char* pwd;
	}CONNECTINFO;
	
	bool io_myssql_query(sql::Connection* conn, const char* sqlStr, std::string& rst
		, QUERYRESULTSETFN queryfn, ERRORFN errfn);
	bool io_mysql_update(sql::Connection* conn, const char* sqlStr
		, UPDATESTMTFN updatefn, ERRORFN errfn);

	bool io_myssql_query(sql::Connection* conn, const char* sqlStr, std::string& rst
		, QUERYRESULTSETFN queryfn);
	bool io_mysql_update(sql::Connection* conn, const char* sqlStr
		, UPDATESTMTFN updatefn);
}

#endif

#endif