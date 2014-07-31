#include <string>
#include <cerrno>
#include <fstream>
#include "file_parse.h"
#include "dad_parse_iterator.h"

#ifndef dad_file_parse_h
#define dad_file_parse_h
using namespace std;

class dad_file_parse :public file_parse<parse_of_dad, char *, dad_parse_iterator>
{
public:
	
	bool open(const wchar_t *filename)
	{
		//调用基类的open函数，唯一的目的，是得到两个数量
		if (file_parse<parse_of_dad, char *, dad_parse_iterator>::open(filename))
		{
			m_stock_count = *(size_t *)(block_ + 8);
			m_quote_count = (size_t)(this->size_ - 16 - 32 - m_stock_count * 32) / 32;
			return true;
		}
		return false;
	}
	iterator begin() const
	{
		if (block_)
			return dad_parse_iterator(block_, block_ + size_, {
			(id_of_dad *)(block_ + 20),
			(quote_of_dad *)(block_ + 16 + 32)
		});
		else
			return dad_parse_iterator();
	}

	iterator end() const
	{
		return dad_parse_iterator();
	}

	size_t m_stock_count=0, m_quote_count=0;

	
};

#endif