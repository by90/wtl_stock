#include <string>
#include <codecvt>
#include <fstream>
#include "sqlite/sqlite3.h"
#include <functional>
#ifndef db_h
#define db_h
class db
{
public:
	static std::string default_;
	static bool set_default(std::string _default,std::function<bool(const char *)> create_database=nullptr)
	{
		default_ = _default;

		//判断文件是否存在，fstream的方法仍然是打开是否正常
		//因此直接略过，用数据库是否正常打开，来判断。
		if (!is_exist(default_))
		{
			if (create_database!=nullptr)
				return create_database(default_.c_str());
			return false;
		}
		return true;

	}
	static bool set_default(std::wstring _default, std::function<bool(std::string)> create_database = nullptr)
	{
		//定义一个转换器
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

		//如果反过来转换:conv.from_bytes(narrowStr);
		return set_default(conv.to_bytes(_default));
	}

private:

	//注意，即使包括一个0字节的同名文件，check也会返回false
	static bool is_exist(std::string _default)
	{
		sqlite3 *db = NULL;
		int rc = 0;
		rc = sqlite3_open_v2(_default.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if (rc != SQLITE_OK)
		{
			sqlite3_close_v2(db);
			return true;
		}
		return false;
	}
};

std::string db::default_="";

#endif