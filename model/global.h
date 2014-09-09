#pragma once
#include <string>
#include "stock_map.h"

#ifdef MODEL_EXPORTS
#define MODEL_API __declspec(dllexport)
#else
#define MODEL_API __declspec(dllimport)
#endif
#include <memory>
#include <vector>
#include <string>  

#ifndef global_h
#define global_h

using namespace std;

////股票市场Enum
//enum MarketEnum
//{
//	ShangHai = 0,
//	ShenZhen = 1,
//	QiHuo = 2
//};
//
////股票类型Enum
//enum CatalogEnum
//{
//	All = 0, //所有，选择的时候使用。
//	Ashares = 1,	//A股
//	SmallAndMedium = 2, //深圳中小板
//	Growth = 3, //深圳创业板
//	Index = 4, //指数：只考虑上证、深证、上证180三只指数，其他不管
//	Bshares = 5, //B股
//	Fund = 6, //基金
//	Bond = 7, //债券
//	Futures = 8, //期货
//	Warrant = 9, //权证
//	Others = 10, //其他	
//};



	MODEL_API bool create_default_database(const char  *str); //创建默认数据库
	MODEL_API const char * GetDefaultDb(); //获取默认数据库路径

	extern MODEL_API StockMap g_stock;
#endif