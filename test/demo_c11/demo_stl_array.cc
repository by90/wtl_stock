#include <array>
#include "gtest/gtest.h"
using namespace std;

TEST(demo,stl_array)
{
	std::array<int, 10> arr;
	int j = 0;

	//注意：auot &i改变每个元素的值
	for (auto &i : arr)
	{
		i = j;
		j++;
	};
	EXPECT_EQ(0, arr[0]);
	EXPECT_EQ(9, arr[9]);
	int *p = arr.data();
	EXPECT_EQ(0, *p);
	EXPECT_EQ(9, *(p + 9));
	std::array<int, 10> *q;
	q = &arr;
	EXPECT_EQ((size_t)10, q->size());
}