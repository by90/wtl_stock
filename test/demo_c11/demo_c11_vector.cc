
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

TEST(DemoC11,vector)
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

	//vector分配一块内存，尾部追加的时候，步骤如下：
	//1.整块内存重新分配
	//2.原来的数据拷贝到新的内存块(memecpy)
	//3.新内存块尾部增加
	//4.释放原来的内存块
	//5.vector的data指向新的地址
	//但为了保证性能，一般新分配的内存块要大一些，这就是
	//当然，若能预知大小，预先resize，则不会出现多次内存分配，性能大幅提高
	
	auto real_size = any_vector.capacity();
	auto object_size = sizeof(any_vector);

	//使用<<:出错时将提供我们需要的信息，不出错不显示
	EXPECT_LE(2u, any_vector.capacity()) << "real_size=" << real_size << "object_size=" << object_size;
	
	//如此，在命令窗口运行test.exe可以看到提示，测试资源管理器运行时看不到。
	std::cout << "capacity=" << real_size << " , object_size=" << object_size;

	//使用reverse重新分配空间,实际的size()不变，能力改变
	any_vector.reserve(10);
	EXPECT_EQ(2, any_vector.size()); //大小不变
	EXPECT_EQ(10, any_vector.capacity());  //存储能力改变

	//使用resize改变大小,size和capacity同时改变。
	any_vector.resize(20);
	EXPECT_EQ(20, any_vector.size()); //大小不变
	EXPECT_EQ(20, any_vector.capacity());  //存储能力改变

	//vector大小一直是16，无论装多少数据，说明内存是heap上分配的，本身只保存指针、大小等。
	//EXPECT_EQ(16, sizeof(any_vector)) << "object_size=" << object_size;

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
