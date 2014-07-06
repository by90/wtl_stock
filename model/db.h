#include <string>
#include <codecvt>
#ifndef db_h
#define db_h
class db
{
public:
	static std::string default_;
	static void set_default(std::string _default)
	{
		default_ = _default;
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