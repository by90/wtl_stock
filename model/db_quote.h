#ifndef db_quote_h
#define db_quote_h

#include <algorithm>
#include "sqlite/sqlite3.h"
#include "db.h"
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
	size_t bulk_insert(T _begin, T _end)
	{
		//这里用static assert,编译器，调用的时候若不是指向结构的指针，则不能通过编译
		//这同样表示：不能编译出不合法的实际函数...起到了与enable_if相似的作用
		static_assert(std::is_same<parse_of_dad, std::iterator_traits<T>::value_type>::value, "Incorrect usage!");
		
		//1.确保数据库打开：
		sqlite3 *default_db=nullptr;
		int rc = sqlite3_open_v2(db::default_.c_str(), &default_db, SQLITE_OPEN_READWRITE, nullptr);
		if (rc != SQLITE_OK)
		{
			sqlite3_close_v2(default_db);
			return 0;
		}
		//此时数据库已经打开，使用default_db
		


		//2.设置sql
		sqlite3_stmt *pStmt = NULL;
		const char *insert_sql = "insert into Quote values(?, ?, ?, ?, ?, ?, ?,?)";

		//3.prepare
		rc = sqlite3_prepare_v2(default_db, insert_sql, -1, &pStmt, 0);
		if (rc)
		{
			fprintf(stderr, "Error: %s\n", sqlite3_errmsg(default_db));
			if (pStmt) sqlite3_finalize(pStmt);
			sqlite3_close_v2(default_db);
			return 0;
		}//此时已经prepare成功

		//4.开启事务
		//这个不需要 needCommit = sqlite3_get_autocommit(defult_db);
		int needCommit = 1;
		if (needCommit) sqlite3_exec(default_db, "BEGIN", 0, 0, 0);//开启事务
		
		//5.循环增加
		int insert_nums = 0;
		for (auto current = _begin; current != _end; ++current)
		{
			sqlite3_bind_text(pStmt, 1, current->idOfDad->id, sizeof(current->idOfDad->id), SQLITE_TRANSIENT);
			sqlite3_bind_int(pStmt, 2, current->quoteOfDad->quoteTime);
			//执行
			sqlite3_step(pStmt);
			rc = sqlite3_reset(pStmt); //重置，下次循环重新执行
			if (rc != SQLITE_OK){
				return 0;
			}
			++insert_nums;			
		}
		//5.提交事务
		
		if (needCommit) sqlite3_exec(default_db, "COMMIT", 0, 0, 0);

		//6.清理现场
		sqlite3_finalize(pStmt);
		sqlite3_close_v2(default_db);
		//7.返回插入的数量
		return insert_nums;
	}
};


#endif