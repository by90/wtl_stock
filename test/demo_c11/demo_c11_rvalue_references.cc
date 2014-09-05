#include "gtest/gtest.h"
using namespace std;

//左值引用
//不支持常量字符串，因为其类型被解析为不同长度的数组
//如果需要支持，则增加T *重载或T[]重载，且不能使用引用，原因是数组本身，不会使用值传递
template <typename T>
bool max(T &first, T &second)
{
	return first > second;
}

//增加这个，可以通过编译
bool max(char *first, char *second)
{

	return strcmp(first,second)>0;
}

template <typename T>
bool max_r(T &&first, T &&second)
{
	return first > second;  
}

template <typename T>
bool max_r(T first[], T  second[])
{
	return strcmp(first, second)>0;
}
//编码转换
TEST(DemoC11, lvalue_refrences)
{
	EXPECT_TRUE(max("2", "1"));

	//这里不能通过编译，因为"2"类型为char[1],"10"类型为char[2]，引用参数中，数组不会蜕化为指针。
	//因此我们增加第二个max，才能应付char *这种
	EXPECT_TRUE(max("2", "10"));

	//右值引用参数，这里同样不能通过编译
	EXPECT_TRUE(max_r("2", "10"));
	EXPECT_TRUE(max_r(3, 2));

}