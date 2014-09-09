#include <string>
#include <vector>
#include <codecvt>
#include <fstream>
#include <functional>
#include <memory>
#include <type_traits> //以上为C++库
#include "sqlite/sqlite3.h" //以上为其他项目的头文件
//次序为C库、C++库、其他项目的头文件、项目内的头文件

//项目名、路径名、头文件名,全部大写
//在#include的后面，让Visual Studio的语法智能感知正常工作
#ifndef STOCK_MODEL_DB_H
#define STOCK_MODEL_DB_H

//Dll项目中，在此定义宏MODEL_API，让Dll项目和使用dll的项目，使用同一个头文件。
//#ifdef MODEL_EXPORTS
//#define MODEL_API __declspec(dllexport)
//#else
//#define MODEL_API __declspec(dllimport)
//#endif

//数据库异常
//类名一般用名词，使用首字母大写形式，不含下划线
class DbException : public std::runtime_error {
public:
	//与连接无关
	DbException(const char *_error_message) : std::runtime_error(_error_message){}

	//与连接有关
	DbException(sqlite3 *_sql_connection) : runtime_error(sqlite3_errmsg(_sql_connection))
	{
		const char *p=this->what();
	}
};

//Blob：
//Thanks For https://github.com/catnapgames/NLDatabase
struct Blob {
public:
	void *data; //结构体成员变量命名，仍然是全部小写，但不使用data_形式
	int length;

	template<typename T>
	Blob(T *data, int length) : data((void*)data), length(length) {}
};

//sql命令类
//单独的使用，主要用于一个sql命令，多次Bind参数的情况
//注意：Query不支持多条命令sqlite3_prepare_v2中的tail为剩下来的语句
class Query
{
public:

	//使用重载，处理ascii或unicode的sql文本，同样在编译期
	Query(std::shared_ptr<sqlite3> _connection, const char sql[]) :connection_(_connection)
	{
		const char *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;

		//char *，其长度使用-1，string则用string的长度，wstring长度要乘以2
		if (sqlite3_prepare_v2(connection_.get(), sql, -1, &stmt_ptr, &tail) != SQLITE_OK)
		{
			//传入连接，返回该连接的最后错误信息，也可获取错误信息后传入字符串
			throw DbException(connection_.get());
		}

		stmt_ = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count_ = sqlite3_column_count(this->stmt_.get());
	}

	//针对wchar_t[]的构造函数
	Query(std::shared_ptr<sqlite3> _connection, const wchar_t sql[]) :connection_(_connection)
	{
		const wchar_t *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		if (sqlite3_prepare16_v2(connection_.get(), sql, -1, &stmt_ptr, (const void**)&tail) != SQLITE_OK)
		{
			throw DbException(connection_.get());
		}
		stmt_ = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count_ = sqlite3_column_count(this->stmt_.get());
	}

	//针对string的构造函数，实际上没必要？
	Query(std::shared_ptr<sqlite3> _connection, std::string &sql) :connection_(_connection)
	{
		const char *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		if (sqlite3_prepare_v2(connection_.get(), sql.c_str(), (int)sql.length(), &stmt_ptr, &tail) != SQLITE_OK)
		{
			throw DbException(connection_.get());
		}
		stmt_ = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count_ = sqlite3_column_count(this->stmt_.get());
	}

	//针对wstring的构造函数
	Query(std::shared_ptr<sqlite3> _connection, std::wstring &sql) :connection_(_connection)
	{
		const wchar_t *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		//注意：wstring的长度要乘以2
		if (sqlite3_prepare16_v2(connection_.get(), sql.c_str(), (int)sql.length() * 2, &stmt_ptr, (const void**)&tail) != SQLITE_OK)
		{
			throw DbException(connection_.get());
		}
		stmt_ = std::shared_ptr<sqlite3_stmt>(stmt_ptr, sqlite3_finalize);
		this->column_count_ = sqlite3_column_count(this->stmt_.get());
	}

	//重置...不需要，使用Excute需要多掌握一个概念。Excute在返回SQLITE_DONE时自动重置。
	//当sql语句为select的时候，第二次Bind再执行，结果会错误
	//因此第二次Bind之前，应Reset，以便清除上次的结果。
	//Insert之类不需要这样。
	//inline void Reset()
	//{
	//	sqlite3_reset(stmt_.get());
	//}

	//重新设置sql语句
	bool Reset(const char *_sql)
	{
		const char *tail = NULL;
		sqlite3_stmt *stmt_ptr = 0;
		//无需sqlites_reset,最新版本在step之后自动的reset了
		sqlite3_reset(stmt_.get());
		//sqlite3_clear_bindings(stmt_.get()); //需要吗？
		//sqlite3_finalize(stmt_.get()); //这里不能手工执行，因为智能指针reset的时候将执行。
		//char *，其长度使用-1，string则用string的长度，wstring长度要乘以2
		if (sqlite3_prepare_v2(connection_.get(), _sql, -1, &stmt_ptr, &tail) != SQLITE_OK)
		{
			//传入连接，返回该连接的最后错误信息，也可获取错误信息后传入字符串
			throw DbException(connection_.get());
			return false;
		}
		stmt_.reset(stmt_ptr, sqlite3_finalize);
		this->column_count_ = sqlite3_column_count(stmt_.get());
		return true;
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
	inline bool  Bind(const int) { return true; }
	
	//sqlite3_bind_int，针对4位以下的整数
	//这里，使用整数、浮点数、其他情况来解包，这样整数版本可以同时处理int,unsigned int等
	//同时，这样的重载，必须保证每个都有限定，否则会出现重载不明确的编译错误
	//我们仅仅在函数返回值中，保留原来的类型，但加一个限制，以便编译器采纳此版本
	template <typename Tint, typename... Args>
	inline typename std::enable_if < std::is_integral<Tint>::value && sizeof(Tint)<8, bool>::type
		Bind(int current, Tint first, const Args &... args)
	{
			if (sqlite3_bind_int(stmt_.get(), current, first) != SQLITE_OK)
			{
				return false;
			}
			return  Bind(current + 1, args...);
	}

	//sqlite3_bind_int64,针对8位以上的整数
	template <typename Tint, typename... Args>
	inline typename std::enable_if <std::is_integral<Tint>::value && sizeof(Tint) >= 8, bool>::type
		Bind(int current, Tint first, const Args &... args)
	{
			if (sqlite3_bind_int64(stmt_.get(), current, first) != SQLITE_OK)
			{
				return false;
			}
			return  Bind(current + 1, args...);
	}

	//sqlite3_bind_double，针对各类浮点数
	template <typename Tdouble, typename... Args>
	inline typename std::enable_if <std::is_floating_point<Tdouble>::value, bool>::type
		Bind(int current, Tdouble first, const Args &... args)
	{
			if (sqlite3_bind_double(stmt_.get(), current, first) != SQLITE_OK)
			{
				return false;
			}
			return  Bind(current + 1, args...);
	}

	//当其他重载都不能识别的时候，转换成字符串然后bind_text
	//写入和读取均需要转换为字符串，构建stringstream临时对象，因此性能较差。
	template <typename T, typename... Args>
	inline typename std::enable_if <!std::is_floating_point<T>::value && !std::is_integral<T>::value, bool>::type
		Bind(int current, T &first, const Args &... args)
	{
			std::stringstream ss;
			ss << first;
			if (sqlite3_bind_text(stmt_.get(), current, ss.str().data(), ss.str().length(), SQLITE_TRANSIENT) != SQLITE_OK)
			{
				return false;
			}
			return  Bind(current + 1, args...);
	}

	//bind const char[]和char *
	//若使用const char *，则传入"first"之类的常量字符串，将不能识别,因为模板函数无法将char[]降级为char *
	//关于长度问题，有的sqlite封装，将size+1，用于容纳最后的0结尾字符，目前暂未发现有何不妥。
	template <typename... Args>
	inline bool Bind(int current, const char first[], const Args &... args)
	{
		if (sqlite3_bind_text(stmt_.get(), current, first, strlen(first), SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return  Bind(current + 1, args...);
	}

	//bind const wchar_t[]
	//长度要乘以2
	template <typename... Args>
	inline bool Bind(int current, const wchar_t first[], const Args &... args)
	{
		if (sqlite3_bind_text16(stmt_.get(), current, first,-1, SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return  Bind(current + 1, args...);
	}

	//bind string
	//使用const char *，则传入"first"之类的常量字符串，将不能识别
	//关于长度问题，有的sqlite封装，将size+1，用于容纳最后的0结尾字符，目前暂未发现有何不妥。
	template <typename... Args>
	inline bool Bind(int current, std::string first, const Args &... args)
	{
		if (sqlite3_bind_text(stmt_.get(), current, first.c_str(), first.length(), SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return  Bind(current + 1, args...);
	}

	//bind wstring
	//长度要乘以2
	template <typename... Args>
	inline bool Bind(int current, std::wstring first, const Args &... args)
	{
		if (sqlite3_bind_text16(stmt_.get(), current, first.c_str(), first.length() * 2, SQLITE_TRANSIENT) != SQLITE_OK)
		{
			return false;
		}
		return  Bind(current + 1, args...);
	}

	//bind Blob
	template <typename... Args>
	inline bool Bind(int current, Blob value, const Args &... args) {
		if (sqlite3_bind_blob(stmt, index, value.data, value.length, SQLITE_STATIC) != SQLITE_OK)
		{
			return false;
		}
		return  Bind(current + 1, args...);
	}

	//读取，中止函数
	inline void ReadColumn(int idx){}

	//整数
	template <typename Tint, typename... Args>
	inline typename std::enable_if < !std::is_enum<Tint>::value && std::is_integral<Tint>::value && sizeof(Tint)<8, void>::type
		ReadColumn(int idx, Tint &first, Args &... args)
	{
			first = sqlite3_column_int(stmt_.get(), idx);
			ReadColumn(idx + 1, args...);
	}

	template <typename Tint, typename... Args>
	inline typename std::enable_if <!std::is_enum<Tint>::value && std::is_integral<Tint>::value && sizeof(Tint) >= 8, void>::type
		ReadColumn(int idx, Tint &first, Args &... args)
	{
			first = sqlite3_column_int64(stmt.get(), idx);
			ReadColumn(idx + 1, args...);
	}

	//如果是enum，需要转换成enum
	template <typename Tenum, typename... Args>
	inline typename std::enable_if <std::is_enum<Tenum>::value, void>::type
		ReadColumn(int idx, Tenum &first, Args &... args)
	{
			first = (Tenum)sqlite3_column_int(stmt.get(), idx);
			ReadColumn(idx + 1, args...);
	}

	template <typename TDouble, typename... Args>
	inline typename std::enable_if <std::is_floating_point<TDouble>::value, void>::type
		ReadColumn(int idx, TDouble &first, Args &... args)
	{
			first = (TDouble)sqlite3_column_double(stmt_.get(), idx);
			ReadColumn(idx + 1, args...);
	}

	//使用const char *，则传入"first"之类的常量字符串，将不能识别
	//关于长度问题，有的sqlite封装，将size+1，用于容纳最后的0结尾字符，目前暂未发现有何不妥。
	template <typename... Args>
	inline void ReadColumn(int idx, std::string& first, Args &... args)
	{
		const char *p = (const char*)sqlite3_column_text(stmt.get(), idx);
		first = string(p, strlen(p) + 1);
		ReadColumn(idx + 1, args...);
	}

	//bind const wchar_t[]
	//长度要乘以2
	template <typename... Args>
	inline void ReadColumn(int idx, std::wstring &first, Args &... args)
	{
		const wchar_t *p = (const wchar_t*)sqlite3_column_text16(stmt.get(), idx);
		first = wstring((wchar_t *)p, wcslen(p) * 2 + 2);
		ReadColumn(idx + 1, args...);
	}

	//使用const char *，则传入"first"之类的常量字符串，将不能识别
	//关于长度问题，有的sqlite封装，将size+1，用于容纳最后的0结尾字符，目前暂未发现有何不妥。
	template <typename... Args>
	inline void ReadColumn(int idx, char first[], Args &... args)
	{
		//int i = sizeof(first); //4字节其实是指针长度
		const char *p = (const char*)sqlite3_column_text(stmt_.get(), idx);
		//int i = strlen(p);
		strcpy_s(first, strlen(p) + 1, p);
		//first = std::string(sqlite3_column_text(stmt.get(), idx), sqlite3_column_bytes(stmt.get(), idx));
		ReadColumn(idx + 1, args...);
	}

	//bind const wchar_t[]
	//长度要乘以2
	template <typename... Args>
	inline void ReadColumn(int idx, wchar_t first[], Args &... args)
	{
		const wchar_t *p = (const wchar_t*)sqlite3_column_text16(stmt_.get(), idx);
		int i = wcslen(p);
		wcscpy_s(first, wcslen(p) + 1, p);
		ReadColumn(idx + 1, args...);
	}
	//SQLITE_STATIC方式 blob
	template <typename... Args>
	inline void ReadColumn(int idx, Blob &first, Args &... args) {
		first = Blob(sqlite3_column_blob(stmt.get(), idx), sqlite3_column_bytes(stmt.get(), idx));
		ReadColumn(idx + 1, args...);
	}

	//执行无返回的Sql命令,如insert、update、delete或create table等。
	//step后应返回Sqlite_done
	inline bool ExcuteNonQuery()
	{
		int rc = sqlite3_step(stmt_.get());
		sqlite3_reset(stmt_.get());
		return (rc == SQLITE_DONE);
	}

	//取得一行结果
	template <typename... Args>
	inline bool Excute(Args & ...args)
	{
		int rc = sqlite3_step(stmt_.get());
		if (rc == SQLITE_ROW)
			ReadColumn(0, args...);
		if (rc==SQLITE_DONE)
			sqlite3_reset(stmt_.get());
		return (rc == SQLITE_ROW);
	}

private:
	std::shared_ptr<sqlite3_stmt> stmt_ = nullptr;
	std::shared_ptr<sqlite3> connection_;
	int column_count_ = 0;
};

class Db
{
	//静态部分
public:
	//这里获得静态变量的指针
	static std::string *default_path()
	{
		static std::string default_path;
		return &default_path;
	}

	static bool set_default_path(const char *_default, std::function<bool(const char *)> create_database = nullptr)
	{
		*default_path() = _default;
		//因此直接略过，用数据库是否正常打开，来判断。
		if (!CheckDatabaseExist(default_path()->c_str()))
		{
			if (create_database != nullptr)
				return create_database(default_path()->c_str());
			return false;
		}
		return true;
	};
	static bool set_default_path(const wchar_t *_default, std::function<bool(const char *)> create_database = nullptr)
	{
		//定义一个转换器
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		auto temp = conv.to_bytes(_default);//如果反过来转换:conv.from_bytes(narrowStr);
		return set_default_path(temp.c_str(), create_database);
	}
	
private:
	static bool CheckDatabaseExist(const char *_default)
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
	std::shared_ptr<sqlite3> connection_ = nullptr;

	Db(const char *filename = nullptr)
	{
		OpenDatabase(filename);
	}

	//这里不能使用=nullptr，否则出现重复的重载
	Db(const wchar_t *filename)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		auto temp = conv.to_bytes(filename);//如果反过来转换:conv.from_bytes(narrowStr);
		OpenDatabase(temp.c_str());
	}

	//是否已连接
	bool operator () ()
	{
		return (connection_.get() != nullptr);
	};

	template <class T>
	Query CreateQuery(T &sql) {
		return Query(this->connection_, sql);
	}

	std::string GetLastError() {
		return std::string((char *)sqlite3_errmsg(connection_.get()));
	}

	inline long long GetLastId() {
		if (!this->connection_.get()) throw DbException("connection_ invalid");
		return sqlite3_last_insert_rowid(connection_.get());
	}

	void SetTimeout(int ms_number) {
		if (!connection_.get()) throw DbException("connection_ invalid");

		if (sqlite3_busy_timeout(connection_.get(), ms_number) != SQLITE_OK)
			throw DbException(connection_.get());
	}

	//执行sql命令，不返回值
	//支持用分号分隔的多条sql语句
	//不支持参数
	bool ExcuteScript(const char *sql)
	{
		sqlite3_stmt *stmt_ptr = 0;
		const char *tail = sql;
		int rc;
		while (tail &&strlen(tail)>0)
		{
			if (sqlite3_prepare_v2(connection_.get(), tail, strlen(tail), &stmt_ptr, &tail) != SQLITE_OK)
			{
				//传入连接，返回该连接的最后错误信息，也可获取错误信息后传入字符串
				throw DbException(connection_.get());
				return false;
			}
			rc = sqlite3_step(stmt_ptr);
			rc = sqlite3_finalize(stmt_ptr);
		}
		return true;
	}


	//或者输出到lambda?
	//执行无返回的单条sql
	//可bind参数
	template <typename... Args>
	bool ExcuteNonQuery(const char *sql, const Args &... args)
	{
		auto query = CreateQuery(sql);
		query.Bind(1, args...);
		return query.ExcuteNonQuery();
	}

	void Begin()
	{
		ExcuteScript("BEGIN");
	}

	void Commit()
	{
		ExcuteScript("COMMIT");
	}
	void Rollback()
	{
		ExcuteScript("ROLLBACK");
	}

private:
	void OpenDatabase(const char *filename = nullptr)
	{
		if (filename == nullptr)
			filename = Db::default_path()->c_str();
		sqlite3 *connection_buffer = nullptr;
		int result = sqlite3_open_v2(filename, &connection_buffer, SQLITE_OPEN_READWRITE, nullptr);  // you can treat errors by throwing exceptions
		if (result == SQLITE_OK)
			connection_ = std::shared_ptr<sqlite3>(connection_buffer, sqlite3_close_v2);
		else
		{
			if (connection_buffer)
				sqlite3_close_v2(connection_buffer);
			throw DbException("unable to open database");
		}
	}
};
#endif