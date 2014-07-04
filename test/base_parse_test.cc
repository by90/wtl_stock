#include "gtest/gtest.h"
#include <functional>
#include "base_parse.h"

//如果要为一组测试(testSuit为blockTest)提供初始化和清理方法，则要定义该testSuit
class BaseParse : public testing::Test {
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

int demoArray[5]{1, 2, 3, 4, 5};
char *demo_ptr = reinterpret_cast<char*>(demoArray);

//默认构造函数，由编译器生成，不用实现代码
TEST_F(BaseParse, construct_default)
{
	base_parse<int *, char *> container;
}

//主要构造函数，传入两个迭代器和一个T类型的move
TEST_F(BaseParse, construct_regular)
{
	//默认构造函数，由编译器生成，不用实现代码
	base_parse<int *, char *> container(demo_ptr, 5, (int *)demo_ptr);
}

//测试begin
TEST_F(BaseParse, begin)
{
	base_parse<int *, char *> container(demo_ptr, 5*sizeof(int), (int *)demo_ptr);
	auto iter = container.begin();
	EXPECT_EQ(1, **iter);
	++iter;
	EXPECT_EQ(2, **iter);
}

//测试end
TEST_F(BaseParse, end)
{
	base_parse<int *, char *> container;
	auto iter = container.end();
	EXPECT_EQ(nullptr, *iter);
}

//应支持range_based loop
TEST_F(BaseParse, should_support_range_based_loop)
{
	base_parse<int *, char *> container(demo_ptr, 5*sizeof(int), (int *)demo_ptr);
	int j = 0;
	for (auto i : container)
	{
		j++;
	}
	EXPECT_EQ(5, j);
}

//应支持range_based loop
TEST_F(BaseParse, should_support_loop_for_default)
{
	base_parse<int *, char *> container;
	int j = 0;
	for (auto i : container)
	{
		j++;
	}
	EXPECT_EQ(0, j);
}