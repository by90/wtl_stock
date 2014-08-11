#include <string>
#include <cerrno>
#include <fstream>
#include <memory>
#include "file_parse.h"
#include "dad_parse_iterator.h"

#ifndef dad_file_parse_h
#define dad_file_parse_h
using namespace std;

class dad_file_parse :public file_parse<parse_of_dad, char *, dad_parse_iterator>
{
public:
	 
	//检查文件是否合法，且获取股票数量和日线数量
	bool check(const wchar_t *filename)
	{
		bool isValidFile = false;
		const long DadFlag = 0x33FC198C;//注意，文件中的4个字节的顺序为8C19FC33，等于0x33fc198c，顺序正好相反。

		long flag = 0;

		std::ifstream reader(filename, std::ios::in | std::ios::binary);
		if (!reader)
		{
			reader.close();
			return false;
		}

		reader.seekg(0, ios::end);//以文件尾定位,beg以文件首字节为0开始定位，cur以当前位置开始定位
		this->size_ =(size_t)reader.tellg();//得到文件大小
		reader.seekg(0, ios::beg); //回到文件第一个字节
		std::shared_ptr<char> ptr(new char[16], [](char* ptr){delete[] ptr; });
		char *buffer = ptr.get();
		reader.read(buffer, 16); //将最初16个字节读入

		flag = *(long *)buffer;
		this->m_stock_count = *(unsigned long *)(buffer + 8);
		this->m_quote_count = (unsigned long)(size_ - 16 - 32 - m_stock_count * 32) / 32;
		bool result=(flag == DadFlag && (((size_ - 16) % 32) == 0));
		
		if (result)
		{

			//此时开始获取最大、最小日期
			int id_line = 0;//最多读3次
			size_t position = 48;
			unsigned long current_date = 0;
			m_start_date = 0;
			m_end_date = 0;
			while (id_line < 3)
			{
				reader.seekg(position, ios::beg); //指向第一条记录
				reader.read((char *)(&current_date), 4);
				if (current_date == -1)
				{
					++id_line;

				}
				else
				{
					if (m_start_date == 0 || m_start_date > current_date)
						m_start_date = current_date;
					if (m_end_date == 0 || m_end_date < current_date)
						m_end_date = current_date;
				}
				position += 32;
				if (position >= size_)
				{
					result = false;
					break;
				}
			}
		}

		
		reader.close();
		return result;
	}
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
	unsigned long m_start_date=0, m_end_date=0;//日线文件中的起止日期，由check判定。

	
};

#endif