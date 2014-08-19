#include <string>
#include <vector>
#include <codecvt>
#include <fstream>
#include "sqlite/sqlite3.h"
#include <functional>
#include <memory>
#include <type_traits>
#ifndef db_h
#define db_h

#ifdef MODEL_EXPORTS
#define MODEL_API __declspec(dllexport)
#else
#define MODEL_API __declspec(dllimport)
#endif

//class DbConnection;

//数据库异常
class DbException : public std::runtime_error {
public:
	//与连接无关
	DbException(const char *error_message) : std::runtime_error(error_message)
	{

	}

	//与连接有关
	DbException(sqlite3 *sql_connection) : runtime_error(sqlite3_errmsg(sql_connection))
	{

	}
};

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
	std::shared_ptr<sqlite3> Connection=nullptr;
	bool Conected = false;	//连接数据库


	DbConnection(const char *filename=nullptr)
	{
		open(filename);
	}

	//这里不能使用=nullptr，否则出现重复的重载
	DbConnection(const wchar_t *filename)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		auto temp = conv.to_bytes(filename);//如果反过来转换:conv.from_bytes(narrowStr);
		open(temp.c_str());
	}

	//是否已连接
	bool operator () ()
	{
		return (Connection.get() != nullptr);
	};


	std::string get_last_error() {
		return std::string((char *)sqlite3_errmsg(Connection.get()));
	}

	long long get_last_id() {
		if (!this->Connection.get()) throw DbException("Connection invalid");
		return sqlite3_last_insert_rowid(Connection.get());
	}

	void DbConnection::set_timeout(int ms_number) {
		if (!Connection.get()) throw DbException("Connection invalid");

		if (sqlite3_busy_timeout(Connection.get(), ms_number) != SQLITE_OK)
			throw DbException(Connection.get());
	}
private:
	void open(const char *filename = nullptr)
	{
		if (filename == nullptr)
			filename = DbConnection::get_default();
		sqlite3 *connection_buffer = nullptr;
		int result = sqlite3_open_v2(filename, &connection_buffer, SQLITE_OPEN_READWRITE, nullptr);  // you can treat errors by throwing exceptions
		if (result == SQLITE_OK)
		{
			Connection = std::shared_ptr<sqlite3>(connection_buffer, sqlite3_close_v2);
		}
		else
		{
			if (connection_buffer)
				sqlite3_close_v2(connection_buffer);
			throw DbException("unable to open database");
			//connection_buffer = nullptr;
		}
	}
};



class DbCommand
{
public:

	//使用重载，处理ascii或unicode的sql文本，同样在编译期
	DbCommand(DbConnection &_connection, char *sql) :connection_(_connection)
	{
		const char *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;

		//char *，其长度使用-1，string则用string的长度，wstring长度要乘以2
		if (sqlite3_prepare_v2(connection_.Connection.get(), sql, -1, &stmt_ptr, &tail) != SQLITE_OK)
		{
			//传入连接，返回该连接的最后错误信息，也可获取错误信息后传入字符串
			throw DbException(connection_.Connection.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}
	DbCommand(DbConnection &connection, wchar_t *sql) :connection_(connection)
	{
		const wchar_t *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		if (sqlite3_prepare16_v2(connection_.Connection.get(), sql, -1, &stmt_ptr, (const void**)&tail) != SQLITE_OK)
		{
			throw DbException(connection.Connection.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}
	DbCommand(DbConnection &connection, std::string &sql) :connection_(connection)
	{
		const char *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		if (sqlite3_prepare_v2(connection.Connection.get(), sql.c_str(), (int)sql.length(), &stmt_ptr, &tail) != SQLITE_OK)
		{
			throw DbException(connection_.Connection.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}
	DbCommand(DbConnection &connection, std::wstring &sql) :connection_(connection)
	{
		const wchar_t *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		//注意：wstring的长度要乘以2
		if (sqlite3_prepare16_v2(connection_.Connection.get(), sql.c_str(), (int)sql.length() * 2, &stmt_ptr, (const void**)&tail) != SQLITE_OK)
		{
			throw DbException(connection_.Connection.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}

	//Bind部分，使用变参模板处理

	inline bool bind(const int) { return true; }
	template <typename T, typename... Args>
	inline bool bind(const int current, const T &first, const Args &... args)
	{
		return DbCommand::bind(this->stmt.get(),current,first,args...);
	}

	/** bind dummy function for empty argument lists **/
	static	bool bind(sqlite3_stmt *statement, const int) { return true; }

	/** bind delegator function that will call a specialized bind_struct **/
	template <typename T, typename... Args>
	static	bool bind(sqlite3_stmt *statement, const int current, const T &first, const Args &... args)
	{
		return bind_struct<T, Args...>::f(statement,current,first, args...);
	}

private:
	std::shared_ptr<sqlite3_stmt> stmt=nullptr;
	DbConnection &connection_;
	int column_count = 0;


	//bind函数，分类型调用：
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
			return DbCommand::bind(statement,current + 1, args...);
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
			return DbCommand::bind(statement, current + 1, args...);
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
			return DbCommand::bind(statement, current + 1, args...);
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
			return DbCommand::bind(statement, current + 1, args...);
		}
	};



};

#endif