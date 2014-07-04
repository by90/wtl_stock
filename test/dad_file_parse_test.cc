#include "gtest/gtest.h"
#include <functional>
#include "dad_file_parse.h"

//如果要为一组测试(testSuit为blockTest)提供初始化和清理方法，则要定义该testSuit
class DadFileParse : public testing::Test {
protected:
	//testSuit的初始化函数
	static void SetUpTestCase() {};

	//testSuit的清理函数
	static void TearDownTestCase() {};

	//SetUpTestCase() 方法在第一个TestCase之前执行
	//TearDownTestCase() 方法在最后一个TestCase之后执行
	//SetUp()方法在每个TestCase之前执行
	//TearDown()方法在每个TestCase之后执行
	//在TestSuit的各测试案例中共享的一些资源，定义为static
};

//默认构造函数，由编译器生成，不用实现代码
TEST_F(DadFileParse, construct_default)
{
	dad_file_parse container;
}

//默认构造的实例，同样能够支持遍历
TEST_F(DadFileParse, shuld_support_loop_for_default)
{
	dad_file_parse container;
	int j = 0;
	//for (auto i : container)
	//{
	//	j++;
	//}
	EXPECT_EQ(0, j);
}

//默认构造函数，由编译器生成，不用实现代码
TEST_F(DadFileParse, open)
{
	dad_file_parse container;
	auto result = container.open("test.dad");
	EXPECT_TRUE(result);
	int j = 0;
	auto p = container.begin();
	EXPECT_STREQ("SH000001", p->idOfDad->id);
	EXPECT_STREQ("上证指数", p->idOfDad->title);
	EXPECT_EQ((long)1402876800, (long)p->quoteOfDad->quoteTime);
	//使用精度比较浮点数
	EXPECT_TRUE(fabs(2070.70f - (*p).quoteOfDad->open) < std::numeric_limits<float>::epsilon());
	EXPECT_FLOAT_EQ(2070.700F, p->quoteOfDad->open);
	EXPECT_FLOAT_EQ(2087.320F, p->quoteOfDad->high);
	EXPECT_FLOAT_EQ(2069.440F, p->quoteOfDad->low);
	EXPECT_FLOAT_EQ(2085.980F, p->quoteOfDad->close);
	EXPECT_FLOAT_EQ(95939768.00F, p->quoteOfDad->volume);
	EXPECT_FLOAT_EQ(79953076224.00F, p->quoteOfDad->amount);

	p++;
	EXPECT_STREQ("SH000001", p->idOfDad->id);
	EXPECT_STREQ("上证指数", p->idOfDad->title);
	EXPECT_EQ((long)1402963200, (long)p->quoteOfDad->quoteTime);

	for (auto i : container)
	{
		j++;
	}
	EXPECT_EQ(9, j);

}