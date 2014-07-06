#include <string>
#include <codecvt>
#include <fstream>
#include "sqlite/sqlite3.h"
#ifndef db_h
#define db_h
class db
{
public:
	static std::string default_;
	static void set_default(std::string _default)
	{
		default_ = _default;

		//判断文件是否存在，fstream的方法仍然是打开是否正常
		//因此直接略过，用数据库是否正常打开，来判断。
		sqlite3 *db = NULL;
		int rc = 0;
		rc = sqlite3_open_v2(default_.c_str(), &db, SQLITE_OPEN_READWRITE , NULL);
		if (rc != SQLITE_OK)
		{
			sqlite3_close_v2(db);
		}

	}
	static void set_default(std::wstring _default)
	{
		//定义一个转换器
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

		//如果反过来转换:conv.from_bytes(narrowStr);
		set_default(conv.to_bytes(_default));
	}
};

std::string db::default_="";

#endif