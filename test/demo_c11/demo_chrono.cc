#include <chrono>
#include "gtest/gtest.h"
using namespace std;

//时间点：两个时间点之间的间隔
TEST(demo, chrono_time_point)
{
	//system_clock：系统时钟,但用户在计时阶段修改了时间，则程序结果会受影响
	//steady_clock：不能被用户修改的时钟，换言之，计时期间即使用户修改了时钟，程序也不受影响。仅在起点使用系统时钟？
	//high_resolution_clock：高精度时钟，实际上是system_clock或steady_clock。

	std::chrono::steady_clock::time_point t1 = std::chrono::system_clock::now();
	std::chrono::steady_clock::time_point t2 = std::chrono::system_clock::now();
	auto countByClock = (t2 - t1).count(); //这种差距为ticks，即时钟周期，比微秒要短
	//EXPECT_GT(i, 0);//大于0
	EXPECT_GE(countByClock, 0);//大于等于0，间隔太短此处为0

	//duration_cast用于将时间的间隔，转换为按某个周期表达的形式，chrono::seconds是预设的周期(1秒)
	auto countBySecond = chrono::duration_cast<std::chrono::seconds>(t2 - t1).count(); //将差距转为秒
	EXPECT_GE(countBySecond, 0);
}

//和ctime间的相互转换
TEST(demo, chrono_and_ctime)
{
	std::time_t firstTime = (std::time_t)1402876800;
	
	//将time_t转化为时间点
	std::chrono::steady_clock::time_point aTime_point=chrono::system_clock::from_time_t(firstTime);

	//time_since_eproch用来获得1970年1月1日到time_point所经过的duration。
	auto real_duration = aTime_point.time_since_epoch().count();
	EXPECT_EQ((long long)1402876800 * 10000000, real_duration); //换句话说，1秒的千万分之一
	auto real_seconds = chrono::duration_cast<std::chrono::seconds>(aTime_point.time_since_epoch()).count();
	EXPECT_EQ((long long)1402876800 , real_seconds); //这样，代表以秒计算，时间点和最初time_t的来源一致
	//也可使用chrono::time_point_cast将时间点转换为按秒计算

	//也可转换回去，重新变成time_t
	std::time_t secondTime = std::chrono::system_clock::to_time_t(aTime_point);
	EXPECT_EQ(firstTime, secondTime);
}

//转换为"2014-1-1"这类形式，用于显示
TEST(demo, chrono_to_string)
{
	std::time_t firstTime = (std::time_t)1402876800;


	//时间转字符串：size_t strftime( char *strDest, size_t maxsize, const char *format, co
	//转换成time_point
	//std::chrono::steady_clock::time_point aTime_point = chrono::system_clock::from_time_t(firstTime);
	//auto first_string=std::put_time(std::localtime(&firstTime), "%Y-%m-%d %X");
	//EXPECT_STREQ("2014-01-01 09:00:00",first_string);
}
