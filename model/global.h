#pragma once
#include <string>

#ifdef MODEL_EXPORTS
#define MODEL_API __declspec(dllexport)
#else
#define MODEL_API __declspec(dllimport)
#endif
#include <memory>
#include <vector>
#include <string>  
#include <hash_map>

#ifndef global_h
#define global_h

using namespace std;

//股票市场Enum
enum MarketEnum
{
	ShangHai = 0,
	ShenZhen = 1,
	QiHuo = 2
};

//股票类型Enum
enum CatalogEnum
{
	All = 0, //所有，选择的时候使用。
	Ashares = 1,	//A股
	SmallAndMedium = 2, //深圳中小板
	Growth = 3, //深圳创业板
	Index = 4, //指数：只考虑上证、深证、上证180三只指数，其他不管
	Bshares = 5, //B股
	Fund = 6, //基金
	Bond = 7, //债券
	Futures = 8, //期货
	Warrant = 9, //权证
	Others = 10, //其他	
};

//日线结构
struct Quote
{
	unsigned long QuoteTime; //使用_mktime32转为__time32t存储，使用_localtime32读取
	float Open, High, Low, Close, Volume, Amount;
};

//权息表结构
struct ExRight
{
	int QuoteTime;
	float AddStock, BuyStock, Price, Bonus;
};


//存放一只股票的代码信息、权息数据和日线
//如果有必要，增加一个日线指针用来存放其所有日线数据
struct Stock
{
	char Id[9]; //代码
	enum MarketEnum Market; //股票代码和市场
	enum CatalogEnum Catalog;  //股票类型
	char Title[9]; //股票名称，10个字节
	char MiniCode[5]; //拼音简码	

	bool ExRightLoaded = false; //权息数据是否载入,按需载入
	bool QuoteLoaded = false; //日线是否载入
	vector<Quote> QuoteSet;
	vector<ExRight> ExRightSet;


	//重载小于和等于操作符，搜索算法需要
	bool operator < (const Stock& a) const
	{
		return (Id <a.Id);
	}
	bool operator == (const Stock& a)const
	{
		return (Id == a.Id);
	}

	//前60天的最高、最低、均价、均量
	//前20天的最高、最低、均价
	//长线分段
	//中线分段
};

namespace global
{
	MODEL_API bool create_default_database(const char  *str); //创建默认数据库
	MODEL_API const char * GetDefaultDb(); //获取默认数据库路径

	extern MODEL_API unsigned long begin_date;
	extern MODEL_API unsigned long end_date; //已保存数据的起止日期
	extern MODEL_API vector<Stock> StockSet;
};
#endif