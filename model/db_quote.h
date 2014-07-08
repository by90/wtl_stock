#pragma once
#include <algorithm>
#include "dad_parse_iterator.h"

struct sqlite3_stmt;

class DbQuote
{
public:
	DbQuote()=default;
	~DbQuote()=default;

	//批量保存quote
	//加入回调函数,回调函数的频次(N条汇报一次，N%汇报一次)
	template <typename T>
	//typedef const std::enable_if<std::is_base_of<std::iterator, T>::value, T>::type 
	size_t bulk_insert(T begin, T end, sqlite3_stmt *pStmt)
	{
		//这里用static assert,编译器，调用的时候若不是指向结构的指针，则不能通过编译
		//这同样表示：不能编译出不合法的实际函数...起到了与enable_if相似的作用
		static_assert(std::is_same<parse_of_dad, std::iterator_traits<T>::value_type>::value, "Incorrect usage!");

		//1.设置sql
		//2.prepare
		//3.开启事务
		//4.循环增加
		for (auto current = begin; current != end; ++current)
		{
			
		}
		//5.提交事务
		//6.返回插入的数量
	}
};


