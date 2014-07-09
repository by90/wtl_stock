
#include <time.h>
namespace stdmore
{
	//将time_t转为tm格式
	//简单封装localtime_s，若需要再修改
	inline errno_t __cdecl to_tm(time_t &from, tm &to)
	{
		return localtime_s(&to, &from);
	}

}