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

	
	//获取日线
	//不要每次打开连接、关闭连接、创建Query
	int GetQuote(int period = 20, std::function<void(const wchar_t *, int)> func = nullptr)
	{
		Db connection;
		Query query = connection.CreateQuery("select QuoteTime, Open, High, Low, Close, Volume, Amount from Quote where id = ? order by QuoteTime");
		Quote quote;
		for (int i = 0; i < g_stock.Data.size(); i++)
		{
			sqlite3_bind_text(query.stmt_.get(), 1, g_stock.Data[i].Id, 9, SQLITE_TRANSIENT);
			do 
			{
				sqlite3_reset(query.stmt_.get());
				quote.QuoteTime = sqlite3_column_int(query.stmt_.get(), 1);
				quote.Open = sqlite3_column_double(query.stmt_.get(), 2);
				quote.Low = sqlite3_column_double(query.stmt_.get(), 3);
				quote.High = sqlite3_column_double(query.stmt_.get(), 4);
				quote.Close = sqlite3_column_double(query.stmt_.get(), 5);
				quote.Volume = sqlite3_column_double(query.stmt_.get(), 6);
				quote.Amount = sqlite3_column_double(query.stmt_.get(), 7);
				g_stock.Data[i].QuoteSet.push_back(quote);
			} while (sqlite3_step(query.stmt_.get()) == SQLITE_ROW);
			if ((i%period) == 0 && (func != nullptr))
				func(nullptr, i * 100 / g_stock.Data.size() + 1);	
			return g_stock.Data.size();
		}		
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
		
	//float BackFactor(vector<Quote> &quotes, ExRight &exright)
	//{
	//  return	(quotes[exright.Start-1].Close*(1.0f + exright.AddStock + exright.BuyStock)) / (quotes[exright.Start-1].Close - exright.Bonus + exright.BuyStock*exright.Price);
	//}

	//wind资讯的涨跌幅复权算法：
	//计算除权因子：针对一条除权信息
	//前复权：复权后价格＝[(复权前价格 - 现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)
	float CaculateFactor(vector<Quote> &quotes, ExRight &exright)
	{
		float last_close_price = quotes[exright.Start - 1].Close; //前收盘价格


		//计算前收盘，按照前复权算法应该为多少
		float adjusted_price = (last_close_price - exright.Bonus + exright.Price*(exright.BuyStock)) / (1.0f + exright.BuyStock + exright.AddStock);


		return last_close_price / adjusted_price; //返回按除权计算的前收盘价与真实前收盘价的比
		//设想一下，前收盘价乘以这个值，得到的是复权价

	}

	void CaculateFactor(vector<Quote> &quotes, vector<ExRight> &exrights)
	{
		float max_factor=-1;
		for (unsigned int i = 0; i < exrights.size(); i++)
		{
			exrights[i].Factor = CaculateFactor(quotes, exrights[i]);
			exrights[i].VolumeFactor = 1 + exrights[i].AddStock + exrights[i].BuyStock;
			if (i>0)
			{
				exrights[i].Factor = exrights[i].Factor *exrights[i - 1].Factor;
				exrights[i].VolumeFactor = exrights[i].VolumeFactor *exrights[i - 1].VolumeFactor;
			}
		}
	}

	//涨跌幅复权
	void AdjustPrice(vector<Quote> &source, vector<ExRight> &exrights)
	{
		GetExOrder(source, exrights);//更新序号
		CaculateFactor(source, exrights);
		int start=0;
		for (unsigned int i = 0;i<exrights.size();i++)
		{
			start = (i>exrights.size() - 1) ? exrights[i - 1].Start : 0;
			for ( int j = exrights[i].Start - 1; j >= start; j--)
			{
				source[j].Open = source[j].Open / exrights[i].Factor;
				source[j].High = source[j].High / exrights[i].Factor;
				source[j].Low = source[j].Low / exrights[i].Factor;
				source[j].Close = source[j].Close / exrights[i].Factor;
				source[j].Volume = source[j].Volume*exrights[i].VolumeFactor;
			}
		}
	}
};


#endif