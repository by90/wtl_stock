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

//Blob：
//Thanks For https://github.com/catnapgames/NLDatabase
class Blob {
public:
	void *data;
	int length;

protected:
	Blob(void *data, int length) : data(data), length(length) {
	}
};


class StaticBlob : public Blob {
public:
	template<typename T>
	StaticBlob(T *data, int length) : Blob((void*)data, length) { }
};


class TransientBlob : public Blob {
public:
	template<typename T>
	TransientBlob(T *data, int length) : Blob((void*)data, length) { }
};


class DbCommand
{
public:

	//使用重载，处理ascii或unicode的sql文本，同样在编译期
	DbCommand(std::shared_ptr<sqlite3> _connection, const char sql[]) :connection_(_connection)
	{
		const char *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;

		//char *，其长度使用-1，string则用string的长度，wstring长度要乘以2
		if (sqlite3_prepare_v2(connection_.get(), sql, -1, &stmt_ptr, &tail) != SQLITE_OK)
		{
			//传入连接，返回该连接的最后错误信息，也可获取错误信息后传入字符串
			throw DbException(connection_.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}
	DbCommand(std::shared_ptr<sqlite3> _connection, const wchar_t sql[]) :connection_(_connection)
	{
		const wchar_t *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		if (sqlite3_prepare16_v2(connection_.get(), sql, -1, &stmt_ptr, (const void**)&tail) != SQLITE_OK)
		{
			throw DbException(connection_.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}
	DbCommand(std::shared_ptr<sqlite3> _connection,  std::string &sql) :connection_(_connection)
	{
		const char *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		if (sqlite3_prepare_v2(connection_.get(), sql.c_str(), (int)sql.length(), &stmt_ptr, &tail) != SQLITE_OK)
		{
			throw DbException(connection_.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}
	DbCommand(std::shared_ptr<sqlite3> _connection, std::wstring &sql) :connection_(_connection)
	{
		const wchar_t *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		//注意：wstring的长度要乘以2
		if (sqlite3_prepare16_v2(connection_.get(), sql.c_str(), (int)sql.length() * 2, &stmt_ptr, (const void**)&tail) != SQLITE_OK)
		{
			throw DbException(connection_.get());
		}
		stmt = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count = sqlite3_column_count(this->stmt.get());
	}

	//Bind部分，使用变参模板处理
	//sqlite3的bind包括如下9个函数
	//int sqlite3_bind_blob(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
	//int sqlite3_bind_double(sqlite3_stmt*, int, double);
	//int sqlite3_bind_int(sqlite3_stmt*, int, int);
	//int sqlite3_bind_int64(sqlite3_stmt*, int, sqlite3_int64);
	//int sqlite3_bind_null(sqlite3_stmt*, int);
	//int sqlite3_bind_text(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
	//int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int, void(*)(void*));
	//int sqlite3_bind_value(sqlite3_stmt*, int, const sqlite3_value*);
	//int sqlite3_bind_zeroblob(sqlite3_stmt*, int, int n);

	//bind参数解析完毕后调用
	inline bool bind(const int) { return true; }


	//sqlite3_bind_int，针对4位以下的整数
	//这里，使用整数、浮点数、其他情况来解包，这样整数版本可以同时处理int,unsigned int等
	//同时，这样的重载，必须保证每个都有限定，否则会出现重载不明确的编译错误
	//我们仅仅在函数返回值中，保留原来的类型，但加一个限制，以便编译器采纳此版本
	template <typename Tint, typename... Args>
	typename std::enable_if <std::is_integral<Tint>::value && sizeof(Tint)<8,bool>::type 
	bind(int current, Tint first, const Args &... args)
	{
		if (sqlite3_bind_int(stmt.get(), current, first) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//sqlite3_bind_int64,针对8位以上的整数
	template <typename Tint, typename... Args>
	typename std::enable_if <std::is_integral<Tint>::value && sizeof(Tint)>=8, bool>::type
		bind(int current, Tint first, const Args &... args)
	{
			if (sqlite3_bind_int64(stmt.get(), current, first) != SQLITE_OK)
			{
				return false;
			}
			return bind(current + 1, args...);
	}

	//sqlite3_bind_double，针对各类浮点数
	template <typename Tdouble, typename... Args>
	typename std::enable_if <std::is_floating_point<Tdouble>::value,bool>::type
	bind(int current,Tdouble first, const Args &... args)
	{
		if (sqlite3_bind_double(stmt.get(), current, first) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//当其他重载都不能识别的时候，转换成字符串然后bind_text
	//写入和读取均需要转换为字符串，构建stringstream临时对象，因此性能较差。
	template <typename T,  typename... Args>
	typename std::enable_if <!std::is_floating_point<T>::value && !std::is_integral<T>::value, bool>::type
	bind(int current, T &first, const Args &... args)
	{
		std::stringstream ss;
		ss << first;
		if (sqlite3_bind_text(stmt.get(), current,ss.str().data(), ss.str().length(),SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//bind const char[]和char *
	//若使用const char *，则传入"first"之类的常量字符串，将不能识别
	//关于长度问题，有的sqlite封装，将size+1，用于容纳最后的0结尾字符，目前暂未发现有何不妥。
	template <typename... Args>
	bool bind(int current, const char first[], const Args &... args)
	{
		if (sqlite3_bind_text(stmt.get(), current, first, strlen(first), SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//bind const wchar_t[]
	//长度要乘以2
	template <typename... Args>
	bool bind(int current, const wchar_t first[], const Args &... args)
	{
		if (sqlite3_bind_text16(stmt.get(), current, first, wcslen(first)*2, SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//bind string
	//使用const char *，则传入"first"之类的常量字符串，将不能识别
	//关于长度问题，有的sqlite封装，将size+1，用于容纳最后的0结尾字符，目前暂未发现有何不妥。
	template <typename... Args>
	bool bind(int current, std::string first, const Args &... args)
	{
		if (sqlite3_bind_text(stmt.get(), current, first.c_str(), first.length(), SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//bind const wchar_t[]
	//长度要乘以2
	template <typename... Args>
	bool bind(int current, std::wstring first, const Args &... args)
	{
		if (sqlite3_bind_text16(stmt.get(), current, first.c_str(), first.length()*2, SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//SQLITE_STATIC方式 blob
	template <typename... Args>
	void bind(int current, StaticBlob value, const Args &... args) {
		if (sqlite3_bind_blob(stmt, index, value.data, value.length, SQLITE_STATIC) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//bind SQLITE_TRANSIENT 形式的Blob
	template <typename... Args>
	void bind(int current, TransientBlob value, const Args &... args) {
		if (sqlite3_bind_blob(stmt, index, value.data, value.length, SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return bind(current + 1, args...);
	}

	//执行无返回的Sql命令
	//step后应返回Sqlite_done
	bool ExecuteNonQuery()
	{
		int rc = sqlite3_step(stmt.get());
		sqlite3_reset(stmt.get());
		return (rc == SQLITE_DONE);
	}

	//取得一行结果
	template <typename... Args>
	bool Execute(Args &... args)
	{
		int rc = sqlite3_step(stmt.get());
		sqlite3_reset(stmt.get());
		return (rc == SQLITE_DONE);
	}

	//支持遍历：begin end和++操作

private:
	std::shared_ptr<sqlite3_stmt> stmt=nullptr;
	std::shared_ptr<sqlite3> connection_;
	int column_count = 0;
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
	std::shared_ptr<sqlite3> Connection = nullptr;
	bool Conected = false;	//连接数据库


	DbConnection(const char *filename = nullptr)
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

	template <class T>
	DbCommand get_command(T &sql) {
		return DbCommand(this->Connection, sql);
	}

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
#endif