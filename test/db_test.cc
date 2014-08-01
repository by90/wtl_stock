
#include "gtest/gtest.h"
#include "db.h"
#include "global.h"
class dbTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		_unlink("ctest.db");
	}

	virtual void TearDown()
	{
		
	}
};

////db类的静态函数set_default
TEST_F(dbTest, set_default_test)
{
	auto rc = db::set_default("ctest.db",global::create_default_database);
	EXPECT_STREQ("ctest.db", db::default());
}

//db类的静态函数set_default使用wstring
TEST_F(dbTest, set_default_wstring_test)
{
	auto rc = db::set_default(L"wtest.db",global::create_default_database);
	EXPECT_STREQ("wtest.db", db::default());
}


