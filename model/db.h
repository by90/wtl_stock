#ifndef db_h
#define db_h
#include <string>
#include <vector>
#include <codecvt>
#include <fstream>
#include "sqlite/sqlite3.h"
#include <functional>
#include <memory>
#ifdef MODEL_EXPORTS
#define MODEL_API __declspec(dllexport)
#else
#define MODEL_API __declspec(dllimport)
#endif

class sql;
class DbConnection
{
//静态部分
public:
	//这里获得静态变量的指针
	static std::string *get_default_pointer()
	{
		static std::string default_;
		return &default_;
	}

	static bool set_default(const char *_default, std::function<bool(const char *)> create_database = nullptr)
	{
		*get_default_pointer() = _default;
		//因此直接略过，用数据库是否正常打开，来判断。
		if (!is_exist(get_default_pointer()->c_str()))
		{
			if (create_database != nullptr)
				return create_database(get_default_pointer()->c_str());
			return false;
		}
		return true;
	};
	static bool set_default(const wchar_t *_default, std::function<bool(const char *)> create_database = nullptr)
	{
		//定义一个转换器
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		auto temp = conv.to_bytes(_default);//如果反过来转换:conv.from_bytes(narrowStr);
		return set_default(temp.c_str(), create_database);
	}
	static const char *get_default(){
		return get_default_pointer()->c_str();
	}

private:
	static bool is_exist(const char *_default)
	{
		sqlite3 *pdb = NULL;
		int rc = 0;
		rc = sqlite3_open_v2(_default, &pdb, SQLITE_OPEN_READWRITE, NULL);
		if (pdb)
			sqlite3_close_v2(pdb);
		return (rc == SQLITE_OK);
	}


//非静态部分
public:
	std::shared_ptr<sqlite3> connection_=nullptr;
	bool conected_ = false;
	//连接数据库


	DbConnection(const char *filename=nullptr)
	{
		initDbConnection(filename);
	}

	//这里不能使用=nullptr，否则出现重复的重载
	DbConnection(const wchar_t *filename)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		auto temp = conv.to_bytes(filename);//如果反过来转换:conv.from_bytes(narrowStr);
		initDbConnection(temp.c_str());
	}

	//是否已连接
	bool operator () ()
	{
		return (connection_ != nullptr);
	};

private:
	void initDbConnection(const char *filename = nullptr)
	{
		if (filename == nullptr)
			filename = DbConnection::get_default();
		sqlite3 *connection = nullptr;
		int result = sqlite3_open_v2(filename, &connection, SQLITE_OPEN_READWRITE, nullptr);  // you can treat errors by throwing exceptions

		if (result == SQLITE_OK)
		{
			connection_ = std::shared_ptr<sqlite3>(connection, sqlite3_close_v2);
		}
		else
		{
			if (connection)
				sqlite3_close_v2(connection);
			connection = nullptr;
		}
	}

//处理bind
public:
	/** sqlite3 statement wrapper for finalization **/
	struct sql_statement {
		//sqlite3_stmt *statement;
		std::shared_ptr<sqlite3_stmt> statement = nullptr;
		sql_statement() : statement(nullptr) {}
		//~sql_statement() { sqlite3_finalize(statement); }
	};

	/** bind dummy function for empty argument lists **/
	static bool bind(sqlite3_stmt *, const int) { return true; }

	/** bind delegator function that will call a specialized bind_struct **/
	template <typename T, typename... Args>
	static bool bind(sqlite3_stmt *statement,
		const int current, const T &first, const Args &... args)
	{
		return bind_struct<T, Args...>::f(statement, current,
			first, args...);
	}

	/** most general bind_struct that relies on implicit string conversion **/
	template <typename T, typename... Args>
	struct bind_struct {
		static bool f(sqlite3_stmt *statement, int current,
			const T &first, const Args &... args)
		{
			std::stringstream ss;
			ss << first;
			if (sqlite3_bind_text(statement, current,
				ss.str().data(), ss.str().length(),
				SQLITE_TRANSIENT) != SQLITE_OK)
			{
				return false;
			}
			return bind(statement, current + 1, args...);
		}
	};

	/** bind_struct for double values **/
	template <typename... Args>
	struct bind_struct<double, Args...> {
		static bool f(sqlite3_stmt *statement, int current,
			double first, const Args &... args)
		{
			if (sqlite3_bind_double(statement, current, first)
				!= SQLITE_OK)
			{
				return false;
			}
			return bind(statement, current + 1, args...);
		}
	};

	/** bind_struct for int values **/
	template <typename... Args>
	struct bind_struct<int, Args...> {
		static bool f(sqlite3_stmt *statement, int current,
			int first, const Args &... args)
		{
			if (sqlite3_bind_int(statement, current, first)
				!= SQLITE_OK)
			{
				return false;
			}
			return bind(statement, current + 1, args...);
		}
	};

	/** bind_struct for byte arrays **/
	template <typename... Args>
	struct bind_struct<std::vector<char>, Args...> {
		static bool f(sqlite3_stmt *statement, int current,
			const std::vector<char> &first, const Args &... args)
		{
			if (sqlite3_bind_blob(statement, current,
				&first[0], first.size(),
				SQLITE_TRANSIENT) != SQLITE_OK)
			{
				return false;
			}
			return bind(statement, current + 1, args...);
		}
	};






};



class MODEL_API DbCommand
{
public:
	sqlite3_stmt *stmt = nullptr;


};
#endif