
#include <vector>
#include "gtest/gtest.h"
using namespace std;

class any_struct
{
public:
	static int construct_number, move_number;
	int first_ = 0, second_ = 0;
	any_struct(int _first, int _second) :first_(_first), second_(_second)
	{
		++construct_number;
	}
	any_struct(any_struct &&_other) :first_(_other.first_), second_(_other.second_)
	{
		++move_number;
	}
};
int any_struct::construct_number = 0;
int any_struct::move_number = 0;

TEST(DemoC11, Vector)
{
	vector<int> any_vector;
	any_vector.push_back(1);
	any_vector.push_back(2);
	any_vector.push_back(3);

	//实际存放了几个元素
	EXPECT_EQ(3, any_vector.size());

	//vector的data()，返回指向T的指针，是连续存放的
	auto pointer = any_vector.data();
	EXPECT_EQ(1, pointer[0]);
	EXPECT_EQ(2, pointer[1]);
	EXPECT_EQ(3, pointer[2]);

	//assign的用法：
	//效果是：先清除vector，再将数组拷贝近来。注意第二个指针是越界的地址。
	int int_array[] = { 4, 5 };
	any_vector.assign(int_array, int_array + 2);
	EXPECT_EQ(2, any_vector.size());
	EXPECT_EQ(4, any_vector[0]);
	EXPECT_EQ(5, any_vector[1]);
	//EXPECT_EQ(3, any_vector[2]);


	//emplace_back，尾部增加
	//是变参语法,参数用来构造一个对象
	//因此是直接构造对象
	any_struct::construct_number = 0;
	any_struct::move_number = 0;
	vector<any_struct> struct_vector;
	struct_vector.emplace_back(10, 20); //构造函数有两个参数

	//emplace_back只执行一次构造，没有移动
	EXPECT_EQ(0, any_struct::move_number); //不移动
	EXPECT_EQ(1, any_struct::construct_number); //构造函数执行一次

	any_struct::construct_number = 0;
	any_struct::move_number = 0;
	struct_vector.push_back(any_struct(30, 40)); //只能预先构造，然后移动
	//push_back执行了1次构造，但多了2次移动
	EXPECT_EQ(2, any_struct::move_number); //移动1次
	EXPECT_EQ(1, any_struct::construct_number); //构造函数执行一次

	//emplace在特定位置之前，增加一个
	//比如在begin处，则新增的出现在begin位置
	//实际上是插入操作
	struct_vector.emplace(struct_vector.begin(), 50, 60);
	EXPECT_EQ(3, struct_vector.size()); //增加了一个
	EXPECT_EQ(50, struct_vector[0].first_);

}
