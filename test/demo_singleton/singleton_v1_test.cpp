
#include <thread>
#include <vector>
#include <algorithm>
#include "singleton_v1.h"
#include "gtest/gtest.h"


TEST(singleton,v1)
{
	auto *first = singleton_v1::GetInstance(1);
	EXPECT_EQ(1, first->id);

	//第二次，即使使用不同参数，由于代码没有执行，返回的仍然是1
	//这意味着只能首次使用赋值
	auto *second = singleton_v1::GetInstance(2);
	EXPECT_EQ(1, first->id);
}

//很明显是线程安全的
TEST(singleton, v1_multi_thread)
{
	//首次执行
	auto p=singleton_v1::GetInstance(1);

	std::vector<std::thread> thread_vector;
	int num = 20;
	for (int n = 0; n < num; ++n) {
		thread_vector.push_back(std::thread([n](int id)
		{
			EXPECT_EQ(n, id); //id即为循环给出的n
			EXPECT_EQ(1,singleton_v1::GetInstance(id)->id);
		}
		, n));
	}

	//等待每个线程结束
	std::for_each(thread_vector.begin(), thread_vector.end(), std::mem_fn(&std::thread::join));
}