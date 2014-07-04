#include "gtest/gtest.h"
#include <functional>
#include "file_parse.h"

//如果要为一组测试(testSuit为blockTest)提供初始化和清理方法，则要定义该testSuit
class FileParse : public testing::Test {
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
TEST_F(FileParse, construct_default)
{
	file_parse<int *, char *> container;
}

//默认构造的实例，同样能够支持遍历
TEST_F(FileParse, shuld_support_loop_for_default)
{
	file_parse<int *, char *> container;
	int j = 0;
	for (auto i : container)
	{
		j++;
	}
	EXPECT_EQ(0, j);
}