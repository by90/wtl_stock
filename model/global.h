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
};

