//头文件数据库访问类

#include "sqlite/sqlite3.h"
#include "db.h"
#include "dad_parse_iterator.h"
#include <vector>


#ifndef db_quote_h
#define db_quote_h

//Global类中包括代码表，而每个代码皆包括权息、财务数据，在第一次使用时载入或启动时一次性载入
//由于财务数据和权息数据整体不足1M，也可考虑作为代码表的Blob字段一次载入，性能或更好
struct code
{
	char id[9];
	char title[9];
	char index[5]; //简拼

	
	//1.函数，获取代码的简拼、市场、交易所、类型
	//2.存放权息、财务数据的vector，需要时载入，或初始化过程中逐步的后台载入

};
class DbCode
{
public:
	DbCode() = default;
	~DbCode() = default;

	//获取全部股票代码
	void GetAllCode()
	{

	}
	//更新某只股票代码：先在内存中是否有、如果有则判断是否一致，再决定是否更新
	//如果没有，insert，如果有，且名称不一致，则update，所有操作先修改内存中的代码表，再更新数据库
	//在内存中搜索...应先find，如果没有即找到插入位置。
	//内存中按照代码顺序排列。
	void Update(const char *id_)
	{

	}

};


#endif