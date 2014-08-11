#ifndef db_h
#define db_h
#include <string>
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

//db实例仅用于连接
//
class MODEL_API db
{
//静态部分
public:
	
	static bool set_default(const char *_default, std::function<bool(const char *)> create_database = nullptr);

	static bool set_default(const wchar_t *_default, std::function<bool(const char *)> create_database = nullptr);
	static const char *default();
private:
	static bool is_exist(const char *_default);
	static std::string default_; //使用default_

public:
	sqlite3 *connection = nullptr;
	//连接数据库
	bool open(const char *filename=db::default())
	{
		int result=sqlite3_open_v2(filename, &connection, SQLITE_OPEN_READWRITE,nullptr);  // you can treat errors by throwing exceptions
		if (result == SQLITE_OK)
		{
			std::shared_ptr<sqlite3> conn(connection, sqlite3_close);
			return true;
		}
		return false;
	}

};

class MODEL_API sql
{
public:
	sqlite3_stmt *stmt = nullptr;


};
#endif