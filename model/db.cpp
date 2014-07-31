#include "stdafx.h"
#define MODEL_EXPORTS
#include "db.h"
#include <stdlib.h>
#include <stdio.h>

std::string db::default_="";

bool db::set_default(const char *_default, std::function<bool(const char *)> create_database)
{
	

	char exeFullPath[MAX_PATH]; // MAX_PATH在WINDEF.h中定义了，等于260
	memset(exeFullPath, 0, MAX_PATH);

	GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);
	char *p = strrchr(exeFullPath, '\\');
	*p = 0x00;

	default_.append(exeFullPath);
	default_.append(_default);
	//default_ =  _default;

	//判断文件是否存在，fstream的方法仍然是打开是否正常
	//因此直接略过，用数据库是否正常打开，来判断。
	if (!is_exist(default_.c_str()))
	{
		if (create_database != nullptr)
			return create_database(default_.c_str());
		return false;
	}
	return true;
};

bool db::set_default(const wchar_t *_default, std::function<bool(const char *)> create_database)
{
	//定义一个转换器
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	auto temp=conv.to_bytes(_default);//如果反过来转换:conv.from_bytes(narrowStr);
    auto rc = set_default(temp.c_str(), create_database);

	//default指针指到temp.c_str()
	//然后temp消失
	//然后default_指向的内容没了
	//然后default_是乱码

	
	return rc;
}
const char *db::default()
{
	return default_.c_str();
}

//注意，即使包括一个0字节的同名文件，check也会返回false
bool db::is_exist(const char *_default)
{
	sqlite3 *pdb = NULL;
	int rc = 0;
	rc = sqlite3_open_v2(_default, &pdb, SQLITE_OPEN_READWRITE, NULL);
	if (pdb)
		sqlite3_close_v2(pdb);
	return (rc == SQLITE_OK);
}
