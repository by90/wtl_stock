//头文件数据库访问类
#include "global.h"
#include "sqlite/sqlite3.h"
#include "db.h"
#include "dad_parse_iterator.h"
#include <vector>

#ifndef db_code_h
#define db_code_h

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

	//获取全部股票代码信息，保存在global::StockSet中
	//时机可在应用启动时，而每只股票相关的日线、权息数据，数据量较大，根据情形按需载入
	//应用初启动的时候，Global::init调用get_date_range获得已安装数据的起止日期，并调用此方法获得代码表
	//安装日线数据时，可考虑凡事大于StockSet中最后日期者，按顺序重新读入
	//读取全部日线，需要2000-3000次的载入操作，每次约2000条数据，大致为600万次操作。
	static void GetAllCode(bool refresh=false)
	{
		if (DbCode::get_stock_list().size()>0)
		{
			if (!refresh)	return;
			else
				DbCode::get_stock_list().clear();
		}
		DbConnection connection;
		auto query = connection.get_command(L"select * from Stock order by Id");
		Stock stock;
		int market, catalog;
		while (query.Execute(stock.Id, market,catalog, stock.Title, stock.MiniCode))
		{
			stock.Market = (MarketEnum)market;
			stock.Catalog = (CatalogEnum)catalog;
			DbCode::get_stock_list().push_back(stock);
		}
		
	}

	static vector<Stock> &get_stock_list()
	{
		static vector<Stock> stock_list;
			return stock_list;
	}

	//更新某只股票代码：先在内存中是否有、如果有则判断是否一致，再决定是否更新
	//如果没有，insert，如果有，且名称不一致，则update，所有操作先修改内存中的代码表，再更新数据库
	//在内存中搜索...应先find，如果没有即找到插入位置。
	//内存中按照代码顺序排列。
	void Update(const char *id_)
	{

	}
	//static vector<Stock> StockList;
};

#endif