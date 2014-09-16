#include <vector>
#include "gtest/gtest.h"
#include "global.h"

std::vector<Quote>   quotes;
std::vector<ExRight> exrights;
//测试向前复权算法
//模拟10条数据,3条权息数据
//为权息数据，计算复权因子和时间段
//复权数据使用实际的数据..测试数据来自实际数据中的几个关键点即前面一条。
TEST(AdjustPrice, CaculateFactor) //计算复权因子
{

}

//除权日，找到第一条小于除权日的记录，还是第一条大于等于除权日的记录？
TEST(AdjustPrice, GetOrder) //为权息记录，获取日线顺序号
{

}

TEST(AdjustPrice, CaculateFactor) //测试前复权结果，包括四个价格、成交量
{

}

//计算流通股本(按照最新流通股和送配股情形)
//流通股本保存在权息数据里
TEST(AdjustPrice, CaculateFactor) 
{

}