
#include <vector>
#include <map>
#ifndef STOCK_MODEL_STOCKAPI_H
#define STOCK_MODEL_STOCKAPI_H
#pragma once

//日线结构
struct Quote
{
	unsigned long QuoteTime; //使用_mktime32转为__time32t存储，使用_localtime32读取
	float Open, High, Low, Close, Volume, Amount;
};
struct ExRight
{
	int QuoteTime;
	float AddStock, BuyStock, Price, Bonus;
};

struct StockInfo
{
	//代码表导入部分
	char Id[9];
	short Market; //股票代码和市场
	short Catalog;  //股票类型
	char Title[9]; //股票名称，4个汉字+0
	char MiniCode[5]; //拼音简码	

	//财务表导入部分
	size_t TotalCirculation; //总股本
	size_t Circulation; //流通盘
	float EarningsPerShare; //每股收益earnings per share

	//权息表导入部分
	std::vector<ExRight> ExRightSet;

	//日线
	std::vector<Quote> QuoteSet;
};
class StockMap
{
public:
	StockMap();
	~StockMap();

	std::vector<StockInfo> Data;
	unsigned long BeginDate = 0, EndDate = 0;

	void SetConnection(); //设置连接和cmd，便于批量处理。
	//启动时载入代码表
	void Load();

	//载入日线
	void LoadQuote(const char *id);

	//如果存在则修改，不存在则加入，保存到数据库另行处理
	void UpdateStock(const char *id,const char *title); //代码表：如果id不存在，加入。如果存在且名称不同，修改名称。如果存在且名称未变，不处理。
	void UpdateBase(const char *id, size_t toatal_circulation,size_t circulation, float earnings_per_share);//财务表：3个相等则不做处理，否则更新。代码不存在不作处理。
	void UpdateExright();//权息表：数量相等且首日、最后一日的日期相等不做处理，否则更新。
	
};

#endif