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
	
};

#endif