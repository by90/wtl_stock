#include <algorithm>
#include "sqlite/sqlite3.h"
#include "db.h"
#include "dad_parse_iterator.h"
#include "global.h"

#ifndef STOCK_MODEL_DB_QUOTE_H
#define STOCK_MODEL_DB_QUOTE_H

struct sqlite3_stmt;

class DbQuote
{
public:
	DbQuote()=default;
	~DbQuote()=default;

	//一次获取，全局
	static MODEL_API long start_date, end_date; //非常量的静态成员，不能初始化

	//获取已安装数据的起止日期
	unsigned long &get_start_date()
	{
		static unsigned long start_date;
		return start_date;
	}

	//获取已安装数据的起止日期
	unsigned long &get_end_date()
	{
		static unsigned long end_date ;
		return end_date;
	}

	//当回调不为为nullptr，则用T *返回结果
	//当回调为nullptr，则直接用T的引用，在参数中返回结果
	//这种体制可以抽象一下...
	void GetSavedDate(unsigned long &start,unsigned long &end, std::function<void(const char *, int)> func = nullptr)
	{
		//获取两个指针，分别指向静态的变量
		//unsigned long start = get_start_date();
		//unsigned long end = get_end_date();
		
		sqlite3 *default_db = nullptr;
		int rc = sqlite3_open_v2(Db::default_path()->c_str(), &default_db, SQLITE_OPEN_READWRITE, nullptr);
	
		//这里仍然应使用异常机制，抛出正常的错误信息，更为便捷。
		//不能恢复的异常，才使用...当数据库不存在，可以创建，连接不存在，可以等待或重新连接，但各处都考虑实在麻烦
		if (rc != SQLITE_OK)
		{
		}

		sqlite3_stmt *pStmt = NULL;
		const char *find_sql = "Select Min(QuoteTime),Max(QuoteTime) From Quote Where Id=?";
		const char *code = "SH000001";
		//3.prepare
		rc = sqlite3_prepare_v2(default_db, find_sql, -1, &pStmt, 0);
		int len = sizeof(code);
		rc=sqlite3_bind_text(pStmt, 1, code,8 , SQLITE_TRANSIENT);
		rc=sqlite3_step(pStmt); //执行
		if (rc != SQLITE_DONE) //只要有数据，返回的就是SQLITE_ROW，如果结束则返回DONE
		{
			//错误
		}
		//SQLITE_NULL

		start= sqlite3_column_int(pStmt, 0);
		end = sqlite3_column_int(pStmt, 1);

		sqlite3_finalize(pStmt);
		sqlite3_close_v2(default_db);

	}

	void delete_all()
	{
		Db connection_;
		try
		{
			auto cmd = connection_.CreateQuery("delete from quote");
			cmd.ExcuteNonQuery();
			cmd.Reset("delete from stock");
			cmd.ExcuteNonQuery();
		}
		catch (DbException e)
		{
			e.what();
		}
		g_stock.Data.clear();
		g_stock.BeginDate = 0;
		g_stock.EndDate = 0;
	}

	//批量保存quote
	//加入回调函数,回调函数的频次(N条汇报一次，N%汇报一次)
	template <typename T>
	//typedef const std::enable_if<std::is_base_of<std::iterator, T>::value, T>::type 
	size_t bulk_insert(T _begin, T _end, int totals = -1, int period = 2000, std::function<void(const wchar_t *, int)> func = nullptr)
	{
		//这里用static assert,编译器，调用的时候若不是指向结构的指针，则不能通过编译
		//这同样表示：不能编译出不合法的实际函数...起到了与enable_if相似的作用
		static_assert(std::is_same<parse_of_dad, std::iterator_traits<T>::value_type>::value, "Incorrect usage!");
		
		//代码表1.创建一个command

		//1.确保数据库打开：
		Db connection_;
		sqlite3 *default_db=connection_.connection_.get(); //使用Api处理quote

		//2.设置sql
		sqlite3_stmt *pStmt = NULL;
		const char *insert_sql = "INSERT OR IGNORE INTO QUOTE VALUES(?, ?, ?, ?, ?, ?, ?,?)";

		//3.prepare
		int rc = sqlite3_prepare_v2(default_db, insert_sql, -1, &pStmt, 0);
		if (rc)
		{
			auto p = sqlite3_errmsg16(default_db);
			func((wchar_t *)p, 0);
			if (pStmt) sqlite3_finalize(pStmt);
			if (default_db)
				sqlite3_close_v2(default_db);
			throw DbException(default_db);
			return 0;
		}//此时已经prepare成功

		//4.开启事务
		//这个不需要 needCommit = sqlite3_get_autocommit(defult_db);
		int needCommit = 1;
		if (needCommit) sqlite3_exec(default_db, "BEGIN", 0, 0, 0);//开启事务
		
		//5.循环增加
		int insert_nums = 0;
		//command不用L""
		auto cmd = connection_.CreateQuery("INSERT OR REPLACE INTO Stock(Id,Market,Catalog,Title,MiniCode) VALUES(?,?,?,?,?)"); //增加或更新代码表命令
		int idNumber = 0;
		id_of_dad *oldId = nullptr;
		StockInfo stock;
		for (auto current = _begin; current != _end; ++current)
		{
			
			//如果代码变化
			if (oldId==nullptr || (oldId!=nullptr && oldId!=current->idOfDad))
			{
				oldId = current->idOfDad;
				//这里过滤代码
				if (g_stock.GetCatalog(current->idOfDad->id) == -1) //不可识别
					continue;//如果代码类型不可识别，即略过后面所有的操作。
				
				//只有代码类别可识别，才在内存代码表里搜索。
				idNumber = g_stock.FindStock(current->idOfDad->id);
				

				if (idNumber < 0 || strcmp(g_stock.Data[idNumber].Title,current->idOfDad->title)!=0) //代码不存在或者虽存在但名称更改
				{
					memcpy(stock.Id,current->idOfDad->id,9);
					memcpy(stock.Title,current->idOfDad->title,9);
					
					//标题的首字母简写都需要修改
					memset(stock.MiniCode, '\0', 5); //全部设为0
					g_stock.GetMiniCode(stock.Title, stock.MiniCode);

					//如果是新增加的股票，此时还需要修改交易所、类型
					if (idNumber < 0)
					{
						stock.Market= g_stock.GetMarket(stock.Id);
						stock.Catalog = g_stock.GetCatalog(stock.Id);

						//新的股票应按顺序加入代码表
						g_stock.Data.insert(g_stock.Data.begin()- (1 * idNumber + 1), stock);
					}
					else //如果仅仅是标题更新，修改即可
					{
						memcpy(g_stock.Data[idNumber].Title, stock.Title, 9);
						
						memcpy(g_stock.Data[idNumber].MiniCode, stock.MiniCode, 5);
					}

					//需要insert或update
					cmd. Bind(1, stock.Id, stock.Market, stock.Catalog, stock.Title, stock.MiniCode);
					cmd.ExcuteNonQuery();
				}

			}
			//保存了16位造成错误？？
			//sqlite3_bind_text(pStmt, 1, current->idOfDad->id, sizeof(current->idOfDad->id), SQLITE_TRANSIENT);
			sqlite3_bind_text(pStmt, 1, current->idOfDad->id, 8, SQLITE_TRANSIENT);
			sqlite3_bind_int(pStmt, 2, current->quoteOfDad->quoteTime);
			sqlite3_bind_double(pStmt, 3, current->quoteOfDad->open);
			sqlite3_bind_double(pStmt, 4, current->quoteOfDad->high);
			sqlite3_bind_double(pStmt, 5, current->quoteOfDad->low);
			sqlite3_bind_double(pStmt, 6, current->quoteOfDad->close);
			sqlite3_bind_double(pStmt, 7, current->quoteOfDad->volume);
			sqlite3_bind_double(pStmt, 8, current->quoteOfDad->amount);
			//执行
			rc = sqlite3_step(pStmt);
			
			if (rc != SQLITE_DONE && rc!=SQLITE_OK){
				auto p = sqlite3_errmsg16(default_db);
				func((wchar_t *)p, 0);
				if (default_db)
					sqlite3_close_v2(default_db);
				throw DbException(default_db);
				return 0;
			}
			sqlite3_reset(pStmt); //重置，下次循环重新执行
			if ((insert_nums%period) == 0 && (func != nullptr))
				func(nullptr, (totals == -1) ? insert_nums : (insert_nums*100 / totals + 1));
			++insert_nums;			
		}
		//5.提交事务
		
		if (needCommit) sqlite3_exec(default_db, "COMMIT", 0, 0, 0);

		//6.清理现场
		sqlite3_finalize(pStmt);
		//sqlite3_close_v2(default_db);

		//7.返回插入的数量
		if (func != nullptr)
			func(nullptr, (totals == -1) ? insert_nums : 100);
		return insert_nums;
	}


	//比较两条日线的大小,返回小于即可
	//注意比较函数需是静态的
	static bool compare_quote(Quote first,Quote second)
	{
		return first.QuoteTime < second.QuoteTime;
	}
	//复权算法
	//返回正数，表示找到，返回值为序号
	//返回负数，表示未找到，返回值为第一个大于该日期的日线序号，也就是登记日。
	//序号-1，为前复权需要计算的地方。
	int GetExOrder(vector<Quote> &quotes,int _date)
	{
		Quote quote;
		quote.QuoteTime = _date;
		std::pair<std::vector<Quote>::iterator, std::vector<Quote>::iterator> bounds;
		bounds = std::equal_range(quotes.begin(), quotes.end(), quote,DbQuote::compare_quote);//返回
		//if (bounds.first == bounds.second)//这是个空区间
		//	return -1 - std::distance(quotes.begin(), bounds.first); //没有找到,返回负值，表示插入的位置，注意，若begin为0，则返回-1
		return (std::distance(quotes.begin(), bounds.first)); //返回找到的第一个与第一个的距离，也就是序号
	}

	//将序号放到权息数组
	void GetExOrder(vector<Quote> &quotes, vector<ExRight> &exrights)
	{
		for (auto &exright : exrights)
		{
			exright.Start = GetExOrder(quotes, exright.QuoteTime);
		}

	}


};


#endif