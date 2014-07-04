#ifndef file_parse_h
#define file_parse_h
#include <string>
#include "base_parse.h"
using namespace std;

template <typename T, typename source_iterator_type = char *>
class file_parse :public base_parse<T,source_iterator_type>
{
public:
	file_parse() = default;
	file_parse(T beginValue) :
		base_parse({}, {}, beginValue)
	{}

	//打开时才读取，因为判断格式合法性无需载入全部文件
	bool open(string _path)
	{
		return false;
	}
protected:
	string path_ = {};
};
#endif