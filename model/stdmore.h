#include <ctime>
#include <string>
#include <codecvt>
#include <stdarg.h>

#ifndef STOCK_MODEL_STDMORE_H
#define STOCK_MODEL_STDMORE_H

namespace stdmore
{
	//localtime_r返回的是指针,localtime_s返回的是错误号typedef int errno_t，两者参数次序亦颠倒。
	//我们使用引用,但何种错误对我们意义不大,简单使用bool返回是否成功即可。
	//不返回tm结构，是减少一次拷贝。不定义一个bool result，减少一次变量分配。inline直接嵌入。
	inline bool  __cdecl localtime(const std::time_t&& from, tm &to)
	{
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		return (localtime_s(&to, &from)==0);
#else
		return (localtime_r(&from, &to)!=nullptr); // POSIX 
#endif
	}

	//将time_t转换为字符串
	//will_replace默认为假，则将结果追加到target，若为真，则返回转换后的字符串。
	inline size_t  __cdecl time_to_wstring(std::time_t &&source,const wchar_t  *format,std::wstring &out_string,bool will_replace=false)
	{
		if (will_replace)
			out_string.clear();
		wchar_t buffer[MAX_PATH] = { 0 };
		std::tm  tm_buffer;
		size_t result;
		stdmore::localtime(std::move(source), tm_buffer);
		result=std::wcsftime(buffer, MAX_PATH - 1, format, &tm_buffer);
		out_string += buffer;
		return result;
	}

	//直接转换为日期
	template <typename charTrait=char>
	inline bool  __cdecl localtime(const std::time_t&& from, std::basic_string<charTrait> &to)
	{
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		return (localtime_s(&to, &from) == 0);
#else
		return (localtime_r(&from, &to) != nullptr); // POSIX 
#endif
	}

	//std::to_string和to_wstring有九个重载，将数值转换为字符串，这是可以日常使用的。

	//将wstring转换为string
	//string之类的跨模块可能有问题？
	inline void __cdecl wstring_to_string(std::wstring from,std::string &to)
	{
		//定义一个转换器
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		to = conv.to_bytes(from);//如果反过来转换:conv.from_bytes(narrowStr);
	}

	//先计算大小，再分配空间？
	//可直接使用MAX_PATH之类的宏...一般也不会超过260字节
	//同时考虑wstring，一般来说，没必要太复杂
	//去掉非标准的代码,仅使用std::sprintf()
	inline void format(std::string& a_string, const char* fmt, ...)
	{
		
		va_list vl;
		va_start(vl, fmt);
		int size = _vscprintf(fmt, vl);
		a_string.resize(++size);
		vsnprintf_s((char*)a_string.data(), size, _TRUNCATE, fmt, vl);
		va_end(vl);
	}
}
#endif