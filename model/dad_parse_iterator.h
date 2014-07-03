#include "parse_iterator.h"
#ifndef dad_parse_iterator_h
#define dad_parse_iterator_h

//Dad文件的证卷代码、名称对照表结构
struct id_of_dad
{
	char id[16];
	char title[16];
};

//Dad文件的行情记录结构
struct quote_of_dad
{
	long quoteTime; //使用_mktime32转为__time32t存储，使用_localtime32读取
	float open, high, low, close;
	float volume, amount;
};

//Dad文件的识别结构
struct parse_of_dad
{
	id_of_dad *idOfDad;
	quote_of_dad *quoteOfDad;
};

//用于识别dad,由此提供parse_of_dad结构
class dad_parse_iterator :public parse_iterator<parse_of_dad>
{
public:
	dad_parse_iterator() = default;
	dad_parse_iterator(char * _begin, char * _end, parse_of_dad _value):
		parse_iterator<parse_of_dad>(_begin, _end, _value)
	{

	}
};


#endif