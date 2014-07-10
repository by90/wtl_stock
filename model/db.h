#ifndef db_h
#define db_h
#include <string>
#include <codecvt>
#include <fstream>
#include "sqlite/sqlite3.h"
#include <functional>
#ifdef MODEL_EXPORTS
#define MODEL_API __declspec(dllexport)
#else
#define MODEL_API __declspec(dllimport)
#endif

class MODEL_API db
{
public:
	
	static bool set_default(const char *_default, std::function<bool(const char *)> create_database = nullptr);

	static bool set_default(const wchar_t *_default, std::function<bool(const char *)> create_database = nullptr);
	static const char *default();
private:
	static bool is_exist(const char *_default);
	static std::string default_; // π”√default_

};
#endif