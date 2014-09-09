#include <unordered_map>
#include <concurrent_unordered_map.h>
#include <unordered_set>
#include "gtest/gtest.h"
using namespace std;

//注意，unordered_map系列是c++ 11标准里的，之前stl的hash_map之类的结构不是c++ 标准的。
TEST(DemoC11, unordered_map)
{
	//定义时，仅指明键值类型
	//其他有默认的参数，查看源码可看到：
	//class _Hasher = hash<_Kty>,
	//class _Keyeq = equal_to<_Kty>,
	unordered_map<std::string,int> int_map;
	int_map.emplace("SH000002", 1);
	EXPECT_EQ(1, int_map.size());

	//EXPECT_EQ(8, int_map.bucket_count()); //多少个"桶"，开始就分配了8个，但这个数字不应是固定的
	int_map["SH000003"] = 3; //以下标方式访问，若不存在，则自动加入
	int_map["SH000001"] = 2;
	EXPECT_EQ(3, int_map.size());

	//EXPECT_EQ(8, int_map.bucket_count());
	
	//安装hash值排序，因几个键类似，外观表现为按加入的顺序排序。
	//注意：不是按键的顺序排序
	//因此，不能直接按顺序输出键，只有另行排序。
	EXPECT_EQ("SH000002", (*(int_map.begin())).first);
	EXPECT_EQ("SH000003", (*(++int_map.begin())).first);
	EXPECT_EQ("SH000001", (*(++++int_map.begin())).first);

	//因此，针对股票代码表
	//1.最多需求：找到某个代码
	//2.次多需求：按id顺序显示：维护一个vector，仅包括id，数据变化后即排序
	//3.换种方式，使用map，查询性能也不错(折半)，同时按顺序维护(二叉树？)

	//所有的这些容器，其主要性能体现在内存分配上
	//因此，预先reverse大小，然后再批量push不超出预先设定的大小，是性能最好的做法。

	//unordered_set和map的不同，是只保存键，不是键值对，也就是集合的概念
	//当然，并非定义了集合的与、或、非操作，仅仅是和unordered_map相似，但没有"值"的容器。
	unordered_set<string> str_set;
	str_set.emplace("second"); 
	str_set.emplace("first");
	str_set.emplace("third");
	//unordered_set<string> second_set;

}