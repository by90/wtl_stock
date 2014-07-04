
#include <vector>
#include <list>
#include "my_advance.h"
#include "gtest/gtest.h"
using namespace std;

//通过my_Advance理解算法和迭代器
TEST(demo, using_traits_in_algorithm)
{
	//1.使用指针：两个int指针间的距离
	int intPtr[]={ 1, 2, 3, 4, 5 };
	int *next_ptr = &intPtr[0];
	my_advance(next_ptr,1);
	EXPECT_EQ(2, *next_ptr);

	//2.随机迭代器
	vector<int> vec = { 1, 2, 3, 4, 5 };
	auto next_vec =  vec.begin();
	my_advance(next_vec, 1);
	EXPECT_EQ(2, *next_vec); //另一个迭代器，等于begin+3，即第0个向后移动3次，则应该为第3个，值为4

	//3.list的双向迭代器
	list<int> list_int = { 1, 2, 3, 4, 5 };
	auto next_list = list_int.begin();
	next_list++;
	next_list++; //该迭代器前移两次，注意，由于不是随机迭代器，不能一次移动两次。
	EXPECT_EQ(3,*next_list);
	my_advance(next_list, 1);
	EXPECT_EQ(4,*next_list);
}
