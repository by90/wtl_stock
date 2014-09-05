#include <memory>
#include "gtest/gtest.h"
using namespace std;

void func(void){
	int *p = new int[5];
	std::shared_ptr<int> p1(p);
	std::shared_ptr<int> p2(p);
	//要使用...，注意编译命令行加上/EHa，避免优化掉try catch模块。
	//此处仅在Test项目|属性|C++|命令行，增加其他命令行
	//离开作用域后会导致两次销毁同一块内存
};
TEST(DemoC11,shared_ptr)
{
	//演示引用计数到销毁的含义
	//这里使用{}来限制作用域
	{
		std::shared_ptr<int> pint;
		EXPECT_EQ(0l, pint.use_count());
		{
			std::shared_ptr<int> pint2(new int[3]);
			EXPECT_EQ(1l, pint2.use_count()); //new int[3]这个指针被引用1次

			pint = pint2;//这时new int[3]这个指针被引用两次
			EXPECT_EQ(2l, pint2.use_count());
			EXPECT_EQ(2l, pint.use_count());
		}//pint2消失，此时，new int[3]的引用计数减一
		EXPECT_EQ(1l, pint.use_count());
	}//此时离开pint作用域，pint消失，new int[3]的引用计数减一，变为0，此时会销毁

	//要注意，不要用两个智能指针指向同一块内存
	//这样，离开作用域的时候会两次销毁，引发异常
	//解决方法：分配后一切基于shared_ptr指针

	//此测试未通过？
	//EXPECT_ANY_THROW(func);
	//Assert::ExpectException

	//多线程情形下，拷贝、赋值操作时，可能由于同时访问引用计数，导致计数无效
	//解决方法：应向每个线程传递公共的weak_ptr，使用时转为shared_ptr

}
