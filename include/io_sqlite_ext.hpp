#ifndef __BYMEDIASDK_SRC_PLATFORM_IO_SQLITE_EXT__
#define __BYMEDIASDK_SRC_PLATFORM_IO_SQLITE_EXT__

#include <sqlite3.h>
#include <string>
#include <vector>
#include <future>

namespace PLATFORM{
	class SqliteTask {
	public:
		SqliteTask(const std::string &db_path) : db_path_(db_path), db_(nullptr) {}
		~SqliteTask() {
			if (db_ != nullptr) {
				sqlite3_close(db_);
				db_ = nullptr;
			}
		}

		int OpenDatabase() {
			return sqlite3_open(db_path_.c_str(), &db_);
		}

		int CloseDatabase() {
			int result = sqlite3_close(db_);
			db_ = nullptr;
			return result;
		}

		int ExecuteSql(const std::string sql) {
			std::string err = "";
			return ExecuteSql(sql, err);
		}

		int ExecuteSql(const std::string& sql, std::string& err) {
			char *errmsg = nullptr;
			int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
			if (result != SQLITE_OK) {
				err = errmsg;
				sqlite3_free(errmsg);
			}
			return result;
		}

		template<typename T>
		std::vector<T> QueryData(const std::string &sql, std::function<void(T& item, sqlite3_stmt *stmt)> fun, std::string& err) {
			std::vector<T> data;
			sqlite3_stmt *stmt = nullptr;
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			if (result != SQLITE_OK) {
				err = "prepare statement failed";
				return data;
			}

			while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
				T item;
				//const unsigned char *name = sqlite3_column_text(stmt, 0);
				//int age = sqlite3_column_int(stmt, 1);
				fun(item, stmt);
				data.push_back(item);
			}

			sqlite3_finalize(stmt);

			if (result != SQLITE_DONE) {
				err = "query data failed";
				return data;
			}

			return data;
		}

	private:
		std::string db_path_;
		sqlite3 *db_;
	};
}
#endif