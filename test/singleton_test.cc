#include <thread>
#include "singleton.h"
#include "gtest/gtest.h"

//测试线程安全性
TEST(SingletonTest,thread_safe_test)
{
	//Singleton<int> first;
	//first.Instance() = 10;

	//Singleton<int> second ;
	//second.Instance() = 20;

	//EXPECT_EQ((int)10, first.Instance());
	
	//std::vector< std::thread > v;
	//int num = 20;

	//for (int n = 0; n < num; ++n) {
	//	v.push_back(std::thread([](int id)
	//	{
	//		auto p=Singleton<int>::Instance();
	//		p = id;
	//	}
	//	, n));
	//}

	//for (auto &iter : v)
	//	iter.join();
	//std::for_each(v.begin(), v.end(), std::mem_fn(&std::thread::join));


}