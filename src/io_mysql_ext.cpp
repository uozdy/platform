#ifdef ENABLE_MYSQL

#include "io_mysql_ext.h"

namespace PLATFORM {
	class CStatementWarp
	{
	public:
		CStatementWarp() : m_pStatement(NULL){}
		~CStatementWarp() {
			if (m_pStatement) delete m_pStatement;
		}

		sql::Statement* createStatement(sql::Connection* conn) {
			m_pStatement = conn->createStatement();
			return m_pStatement;
		}
	private:
		sql::Statement* m_pStatement;
	};

	class CPreparedStatementWarp
	{
	public:
		CPreparedStatementWarp() : m_pStatement(NULL){}
		~CPreparedStatementWarp() {
			if (m_pStatement) delete m_pStatement;
		}

		sql::PreparedStatement* prepareStatement(sql::Connection* conn, const char* sql) {
			m_pStatement = conn->prepareStatement(sql);
			return m_pStatement;
		}
	private:
		sql::PreparedStatement* m_pStatement;
	};

	sql::Connection* io_mysql_getconnection(CONNECTINFO* info)
	{
#if 0
		std::unique_lock<std::mutex> lock(s_instanceMtx);
		if (s_sqlConn) {
			if (s_sqlConn->isClosed() || !s_sqlConn->isValid()) {
				s_sqlConn->reconnect();
			}
			return s_sqlConn;
		}
#endif
		if (!info) return NULL;

		sql::Connection* sqlConn = NULL;
		sql::Driver* sqlDriver = get_driver_instance();
		try {
			sqlConn = sqlDriver->connect(info->host, info->user, info->pwd);
		}
		catch (...) {
			printf("connect failed\n");
		}
		return sqlConn;
	}

	bool io_myssql_query(sql::Connection* conn, const char* sqlStr, std::string& rst
			, QUERYRESULTSETFN queryfn, ERRORFN errfn)
	{
		if (!conn) return false;

		try {
			CStatementWarp stmtwarp;
			sql::Statement* stmt = stmtwarp.createStatement(conn);
			sql::ResultSet* res = stmt->executeQuery(sqlStr);

			while (res->next()) {
				std::string item = queryfn(res);
				rst.append(item.c_str());
			}

			return true;
		}
		catch (sql::SQLException e) {
			return errfn(e);
		}
		catch (...) {
		}

		return false;
	}

	bool io_mysql_update(sql::Connection* conn, const char* sqlStr
		, UPDATESTMTFN updatefn, ERRORFN errfn)
	{
		if (!conn) return false;

		try {
			CPreparedStatementWarp statewarp;
			sql::PreparedStatement* stmt = statewarp.prepareStatement(conn, sqlStr);
			updatefn(stmt);

			int updatecount = stmt->executeUpdate();
			if (updatecount) {
				return true;
			}
		}
		catch (sql::SQLException e) {
			return errfn(e);
		}
		catch (...) {
		}

		return false;
	}

	bool io_myssql_query(sql::Connection* conn, const char* sqlStr, std::string& rst
		, QUERYRESULTSETFN queryfn)
	{
		return io_myssql_query(conn, sqlStr, rst, queryfn
			, [](sql::SQLException& err)->bool {return false; });
	}

	bool io_mysql_update(sql::Connection* conn, const char* sqlStr
		, UPDATESTMTFN updatefn)
	{
		return io_mysql_update(conn, sqlStr, updatefn
			, [](sql::SQLException& err)->bool {return false; });
	}
}

#endif