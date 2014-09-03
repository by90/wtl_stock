
#include "gtest/gtest.h"
#include "db.h"
#include "global.h"

bool create_demo_database(const char * path)
{
	sqlite3 *db = NULL;
	int rc = sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	char * pErrMsg = 0;

	rc = sqlite3_exec(db,
		"PRAGMA page_size=4096;"
		"CREATE TABLE if not exists [Product] ("
		"[ID] INTEGER PRIMARY KEY AUTOINCREMENT,"
		"[TITLE] nchar[20] NOT NULL,"
		"[WTITLE] nchar[20] NOT NULL,"
		"[DATE] INTEGER,"		
		"[PRICE] FLOAT,"
		"[NUMBER] INTEGER,"
		"[IMAGE] Blob); ",
		0, 0, &pErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("Error message: %s\n", pErrMsg);
		sqlite3_free(pErrMsg);//如果有错误，则需要free返回的错误信息
		sqlite3_close_v2(db);
		return false;
	}
	else
	{
		sqlite3_close_v2(db);
		return true;
	}
}
class dbTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		_unlink("ctest.db");
		_unlink("wtest.db");
		_unlink("test.db");
		Db::set_default_path("ctest.db", create_demo_database);
		Db conn;

		//共有6个字段，如果只书写5个字段会触发异常
		Query cmd = conn.create_query("INSERT INTO PRODUCT VALUES (?,?,?,?,?,?,?)");

		//从2开始bind，因为第一个是自增长字段
		//只bind 4个参数，最后一个blob字段没有bind
		cmd.bind(2, "first", "第一个", (unsigned long long)1402876800, (double)100.10, (int)10);
		cmd.excute_non_query();

		cmd.bind(2, "second", L"第二个", (int)1402876800, (float)200.20, (int)20);
		cmd.excute_non_query();

		std::string first_string = "third";
		std::wstring first_wstring = L"第三个";
		cmd.bind(2, first_string, first_wstring, (int)1402876800, (float)300.20, (int)30);
		cmd.excute_non_query();
	}

	virtual void TearDown()
	{
		
	}
};

////db类的静态函数set_default
TEST_F(dbTest, set_default_test)
{
	auto rc = Db::set_default_path("ctest.db",create_demo_database);
	EXPECT_STREQ("ctest.db", Db::default_path()->c_str());
}

//db类的静态函数set_default使用wstring
TEST_F(dbTest, set_default_wstring_test)
{
	auto rc = Db::set_default_path(L"wtest.db",create_demo_database);
	EXPECT_STREQ("wtest.db", Db::default_path()->c_str());
}

//db类构造函数
TEST_F(dbTest, db_construct_test)
{
	//文件不存在，应抛出异常
	EXPECT_ANY_THROW(Db db_not_exist("file"));
		 //file数据库不存在
	//EXPECT_FALSE(db_not_exist());
	//再次确认创建了ctest.db
	Db::set_default_path("ctest.db", global::create_default_database);
	Db db_exist("ctest.db");
	EXPECT_TRUE(db_exist());

	Db db_empty;
	EXPECT_TRUE(db_empty());
}

//db类构造函数
TEST_F(dbTest, db_wchar_construct_test)
{
	//文件不存在应抛出异常
	EXPECT_ANY_THROW(Db db_not_exist(L"file"));

	//再次确认创建了ctest.db
	Db::set_default_path(L"wtest.db", global::create_default_database);
	Db db_exist(L"wtest.db");
	EXPECT_TRUE(db_exist());

	Db db_empty;
	EXPECT_TRUE(db_empty());
}

//char数组有一位"/0",wchar_t数组有两位
TEST_F(dbTest, wchar_array_test)
{
	wchar_t first[] = L"你好";
	EXPECT_EQ(6, sizeof(first)); //wchar数组的长度为6，结束符为两个0
}

struct Product
{
	int id;
	char title[21];
	wchar_t wtitle[11];
	int date;
	float price;
	int number;
	//image 暂时不列入
};
//db类构造函数
TEST_F(dbTest, db_insert_test)
{
	//再次确认创建了ctest.db
	Db::set_default_path("ctest.db", create_demo_database);
	Db conn;
	Query query = conn.create_query("SELECT COUNT(*) FROM PRODUCT");
	int count = 0;
	query.excute(count);
	EXPECT_EQ(3,count);
}

//获取一行的所有内容
//注意覆盖了int、float、string等，则测试了输出一行结果的各种类型
TEST_F(dbTest, db_query_test)
{
	Db::set_default_path("ctest.db", create_demo_database);
	Db conn;
	Product product;
	Query row_cmd = conn.create_query("SELECT * FROM PRODUCT");
	//while (row_cmd.excute(product.id, product.title, product.wtitle, product.date, product.price, product.number))
	//{
	//	int i=product.id;
	//}
	char wtitle_ptr[11] = { 0 };
	row_cmd.excute(product.id, product.title, wtitle_ptr, product.date, product.price, product.number);

	//注意，char[]表达的汉字，写入数据库，可以char[]正确读出。在数据库中使用普通管理工具，为乱码，无关紧要。
	EXPECT_STREQ("第一个", wtitle_ptr);
}

//Db类应提供一个始终打开的连接
TEST_F(dbTest, db_default_connection_)
{
	//auto conn = Db::GetDb(); //返回默认的连接
	//EXPECT_TRUE(conn());
}




