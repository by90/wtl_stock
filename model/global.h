#pragma once
#include <string>

#ifdef MODEL_EXPORTS
#define MODEL_API __declspec(dllexport)
#else
#define MODEL_API __declspec(dllimport)
#endif

namespace global
{
	MODEL_API bool create_default_database(const char  *str);
	MODEL_API const char * GetDefaultDb();
	extern MODEL_API unsigned long begin_date;
	extern MODEL_API unsigned long end_date; //已保存数据的起止日期
};

