#include <fstream>
#include "sqlite/sqlite3.h"

#include "global.h"
#include "db.h"


// 读取大智慧导出的财务数据
//文件中还包括香港及其他股市的数据
// 大智慧财务数据文件结构是这样的，
// 文件前8个字节是文件头，财务数据记录从第9个字节开始。
// 9－16个字节是股票代码如：sh000001，然后空4个字节，然后是发布日期（4个字节）
// 报告期（4个字节） 上市日期（4个字节），后面的都是4个字节  依次是  每股收益
// 每股净资产   净资产收益率   每股经营现金  每股公积金  每股未分配  股东权益比
// 净利润同比  主营收入同比  销售毛利率  调整每股净资产  总资产  流动资产 固定资产
// 无形资产  流动负债  长期负债  总负债   股东权益  资本公积金  经营现金流量
// 投资现金流量   筹资现金流量  现金增加额   主营收入 主营利润  营业利润
// 投资收益   营业外收支  利润总额    净利润  未分配利润  总股本  无限售股合计
// A股  B股  境外上市股  其他流通股   限售股合计   国家持股  国有法人股   境内法人股
// 境内自然人股  其他发起人股   募集法人股    境外法人股  境外自然人股   优先股或其他 
// 然后开始下一条记录
// 每条记录共148个字节。
//这里映射每只股票在财务文件中的描述

//51X4+12=216字节
struct FinanceFull
{
	char Id[9]; //代码，8位+0;
	char NoUse[3]; //实际上代码占了12位，后面3个0略去
	int32_t PublishDate; //发布日期...实际上是生成财务文件的日期
	int32_t ReportDate; //报告日期...每个季度的季尾日
	int32_t StartDate; //该股票上市日期
	float EarningsPerShare; //每股收益
	float NetAssetPerShare; //每股净资产
	float ROE; //净资产收益率
	float OperatingCashPerShare; //每股经营现金
	float ShareFund; //每股公积金
	float UnallocatedShare; //每股未分配
	float EquityRatio; //股东权益比
	float TheSamePeriodNetProfitRatio; //净利润同比
	float TheSamePeriodMainIncomeRaio; //主营收入同比
	float SalesGrossProfitRate; //销售毛利率
	float AdjustNetAssetPerShare; //调整每股净资产
	float TotalAssets; //总资产
	float CurrentAssets; //流动资产
	float FixedAssets; //固定资产
	float IntangibleAssets; //无形资产
	float CurrentLiabilities; //流动负债
	float LongTermLiabilities; //长期负债
	float TotalLiabilities; //总负债
	float Equity; //股东权益
	float CapitalFund; //资本公积金
	float OperatingCashFlow; //经营现金流量
	float CashInvestmentFlow;//投资现金流量

	float FinancingCashFlow;//筹资现金流量
	float IncreaseInCash; //现金增加额
	float MainIncome;//主营收入
	float CoreProfit; //主营利润
	float OperatingProfit; //营业利润
	float InvestmentIncome; //投资收益
	float UnrelatedBusinessIncome; //营业外收支
	float TotalProfit; //利润总额
	float NetProfit; //净利润
	float UndistributedProfit; //未分配利润
	float TotalEquity; //总股本
	
	//我们将这个，加入到代码表，总流通股份。
	//注意，单位是"万股"，浮点数乘以10000才是实际的流通股。
	float TotalUnlimitedShares; //无限售股合计

	float AShares; //A股
	float BShares; //B股
	float OverseasShares; //境外上市股
	float OtherShares; //其他流通股
	float TotalRestrictedShares; //限售股合计
	float StateOwnership; //国家持股
	float StateLegalPersonShares; //国有法人股
	float DomesticLegalPersonShares; //境内法人股	
	float DomesticNaturalPersonShares; // 境内自然人股
	float OtherPromotersShares; //其他发起人股
	float RaiseCorporateShares; //募集法人股
	float ForeignLegalPersonShares; //境外法人股
	float ForeignNaturalPersonShares; //境外自然人股
	float PreferenceSharesOrOther; //优先股或其他
};
//注意：权息数据从新到旧顺序排列
class DbFinance
{
public:

	//读取...
	//当返回的vector长度为0,则不合法

	//insert使用vector的begin end迭代器

	bool check(const wchar_t *filename)
	{
		bool isValidFile = false;
		const long ExrightFlag = 0x223FD90D;
		size_t size = 0;
		long flag = 0;

		std::ifstream reader(filename, std::ios::in | std::ios::binary);
		if (!reader)
		{
			reader.close();
			return false;
		}

		reader.seekg(0, ios::end);//以文件尾定位,beg以文件首字节为0开始定位，cur以当前位置开始定位
		size = (size_t)reader.tellg();//得到文件大小
		reader.seekg(0, ios::beg); //回到文件第一个字节
		std::shared_ptr<char> ptr(new char[8], [](char* ptr){delete[] ptr; });
		char *buffer = ptr.get();
		reader.read(buffer, 8); //将最初8个字节读入

		flag = *(long *)buffer;

		return (flag == ExrightFlag);

	}

	size_t bulk_insert(const wchar_t *_file, int totals = -1, int period = 50, std::function<void(const wchar_t *, int)> func = nullptr)
	{

		if (g_stock.Data.size() <= 0)
		{
			func(L"代码表没有安装，此次安装财务数据失败!", 100);
			return 0;
		}
		//读取文件
		std::ifstream reader;  //注意，来自<fstream>而非<iostream>
		reader.open(_file, std::ios::binary | std::ios::in);  //以二进制只读方式打开文件
		reader.seekg(0, std::ios::end);//以文件尾定位,beg以文件首字节为0开始定位，cur以当前位置开始定位
		long fileSize = (long)reader.tellg();//得到文件大小
		reader.seekg(0, std::ios::beg); //回到文件第一个字节
		std::shared_ptr<char> ptr(new char[fileSize], [](char* ptr){delete[] ptr; });
		reader.read(ptr.get(), fileSize); //将全部文件读取到内存
		reader.close(); //关闭文件


		FinanceFull *position = (FinanceFull *)(ptr.get() + 8);

		int total_number = (fileSize - 8) / sizeof(FinanceFull); //一共多少条记录
		int id_number = 0;

		Db conn;
		conn.Begin();
		auto cmd = conn.CreateQuery("UPDATE STOCK SET Circulation=? WHERE ID=?");
		int order = -1;
		for (int i = 0; i < total_number;i++)
		{
			order = g_stock.FindStock(position->Id);
			
			if (order>0 && g_stock.Data[order].Catalog!=4) //不需要考虑指数
			{
				g_stock.Data[order].Circulation = position->TotalUnlimitedShares;
				cmd.Bind(1, g_stock.Data[order].Circulation,position->Id);
				cmd.ExcuteNonQuery();
			}
			if ((i % period) == 0)
			{
				func(nullptr, i * 80 / total_number);
			}
			position++;
		}

		conn.Commit();
		//func(nullptr, 100);
		return total_number;
		//写入所有权息数据
	}

};