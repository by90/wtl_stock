
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
	}

	virtual void TearDown()
	{
		
	}
};

////db类的静态函数set_default
TEST_F(dbTest, set_default_test)
{
	auto rc = DbConnection::set_default("ctest.db",create_demo_database);
	EXPECT_STREQ("ctest.db", DbConnection::get_default());
}

//db类的静态函数set_default使用wstring
TEST_F(dbTest, set_default_wstring_test)
{
	auto rc = DbConnection::set_default(L"wtest.db",create_demo_database);
	EXPECT_STREQ("wtest.db", DbConnection::get_default());
}

//db类构造函数
TEST_F(dbTest, db_construct_test)
{
	//文件不存在，应抛出异常
	EXPECT_ANY_THROW(DbConnection db_not_exist("file"));
		 //file数据库不存在
	//EXPECT_FALSE(db_not_exist());
	//再次确认创建了ctest.db
	DbConnection::set_default("ctest.db", global::create_default_database);
	DbConnection db_exist("ctest.db");
	EXPECT_TRUE(db_exist());

	DbConnection db_empty;
	EXPECT_TRUE(db_empty());
}

//db类构造函数
TEST_F(dbTest, db_wchar_construct_test)
{
	//文件不存在应抛出异常
	EXPECT_ANY_THROW(DbConnection db_not_exist(L"file"));

	//再次确认创建了ctest.db
	DbConnection::set_default(L"wtest.db", global::create_default_database);
	DbConnection db_exist(L"wtest.db");
	EXPECT_TRUE(db_exist());

	DbConnection db_empty;
	EXPECT_TRUE(db_empty());
}

//db类构造函数
TEST_F(dbTest, db_insert_test)
{
	//再次确认创建了ctest.db
	DbConnection::set_default("ctest.db", create_demo_database);

	DbConnection conn;

	//共有6个字段，如果只书写5个字段会触发异常
	DbCommand cmd(conn, "INSERT INTO PRODUCT VALUES (?,?,?,?,?,?)");
	cmd.bind(2, "first", (int)1402876800,(double)12.10,(int)200);
}