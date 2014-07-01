//gtest和gmock测试的示例
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using testing::Return;

TEST(DemoTest, JustDemo)
{
	EXPECT_EQ(2, 1+1);
}

//以下是gmock使用的例子
//接口类：为方便起见，我们不实现Vistor，仅提供一个接口count,表示目前访问的数量
class Vistor {
public:
	virtual int count() = 0;
};

//mock一个Vistor
class MockVistor : public Vistor {
public:
	//如此即定义了一个mock的函数
	MOCK_METHOD0(count, int());
};

//测试
TEST(MockDemoTest, mockDemo) {
//实例化一个mock的对象
MockVistor vistor;
//声明将2次，每次返回什么值，注意，如果测试中没有调用两次，测试会不能通过
EXPECT_CALL(vistor,count()).Times(2).WillOnce(Return((int)100)).WillRepeatedly(Return((int)200));
//第一次调用，应返回100
int count = vistor.count();
EXPECT_EQ(count,(int)100);
//第二次调用，应返回200
count = vistor.count();
EXPECT_EQ(count, (int)200);
}
