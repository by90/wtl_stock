//注意：Db.h的默认数据库路径，是局部静态变量，在model中访问时，需要用这种单一头文件形式

#include <vector>
#include <algorithm>

#include "db.h"

#ifndef STOCK_MODEL_STOCKAPI_H
#define STOCK_MODEL_STOCKAPI_H
#pragma once
//
//#ifdef MODEL_EXPORTS
//#define MODEL_API __declspec(dllexport)
//#else
//#define MODEL_API __declspec(dllimport)
//#endif

//日线结构
struct Quote
{
	unsigned long QuoteTime; //使用_mktime32转为__time32t存储，使用_localtime32读取
	float Open, High, Low, Close, Volume, Amount;
};
struct ExRight
{
	int QuoteTime;
	float AddStock, BuyStock, Price, Bonus;
};

struct StockInfo
{
	//代码表导入部分
	char Id[9];
	short Market; //股票代码和市场
	short Catalog;  //股票类型
	char Title[9]; //股票名称，4个汉字+0
	char MiniCode[5]; //拼音简码	

	//财务表导入部分
	size_t TotalCirculation; //总股本
	size_t Circulation; //流通盘
	float EarningsPerShare; //每股收益earnings per share

	//权息表导入部分
	std::vector<ExRight> ExRightSet;

	//日线
	std::vector<Quote> QuoteSet;

	//重载小于和等于操作符，搜索算法需要
	bool operator < (const StockInfo& a) const
	{
		return (strcmp(Id, a.Id)<0);
	}
	bool operator == (const StockInfo& a)const
	{
		return (strcmp(Id, a.Id) == 0);
	}
};
class StockMap
{
public:
	std::vector<StockInfo> Data;
	unsigned long BeginDate = 0, EndDate = 0;

	void SetConnection(); //设置连接和cmd，便于批量处理。

	//载入日线
	void LoadQuote(const char *id);

	//如果存在则修改，不存在则加入，保存到数据库另行处理
	void UpdateStock(const char *id,const char *title); //代码表：如果id不存在，加入。如果存在且名称不同，修改名称。如果存在且名称未变，不处理。
	void UpdateBase(const char *id, size_t toatal_circulation,size_t circulation, float earnings_per_share);//财务表：3个相等则不做处理，否则更新。代码不存在不作处理。
	void UpdateExright();//权息表：数量相等且首日、最后一日的日期相等不做处理，否则更新。
	
	StockMap()
	{
	}


	~StockMap()
	{
	}

	//如返回值result小于0，表示没有找到，-1*result-1为插入位置
	int FindStock(const char id[9])
	{
		StockInfo stock;
		strcpy_s(stock.Id, id); //拷贝
		std::pair<std::vector<StockInfo>::iterator, std::vector<StockInfo>::iterator> bounds;
		bounds = std::equal_range(Data.begin(), Data.end(), stock);//返回
		if (bounds.first == bounds.second)
			return -1 - std::distance(Data.begin(), bounds.first); //没有找到,返回负值，表示插入的位置，注意，若begin为0，则返回-1
		return (std::distance(Data.begin(), bounds.first)); //返回找到的第一个与第一个的距离，也就是序号
	}

	void Load(bool refresh=false)
	{
		if (Data.size()>0)
		{
			if (!refresh)	return;
			else
				Data.clear();
		}
		Db connection;
		auto query = connection.CreateQuery(L"select * from Stock order by Id");
		StockInfo stock;
		while (query.Excute(stock.Id, stock.Market, stock.Catalog, stock.Title, stock.MiniCode, stock.TotalCirculation, stock.Circulation, stock.EarningsPerShare))
		{
			Data.push_back(stock);
		}
	}

	//根据代码获取交易所
	short int GetMarket(const char id[])
	{
		return (id[1] == 'H') ? 0 : (id[1] == 'Z') ? 1 : 2;
	}

	//根据代码，获取股票类型，"其它"类型将被忽略
	//#include <regex> 正则表达式，效率可能略低
	short int GetCatalog(const char id[])
	{
		//判断上证
		if (memcmp("SH6", id, 3) == 0)
			return 1;

		//先判断深圳中小板
		if (memcmp("SZ002", id, 5) == 0)
			return 2;

		//再判断深圳创业板
		if (memcmp("SZ300", id, 5) == 0)
			return 3;

		//然后深圳A股
		if (memcmp("SZ00", id, 4) == 0)
			return 1;

		//四个指数
		if (memcmp(id,"SH000001",8)==0 || memcmp(id,"SZ399001",8)==0 || memcmp(id,"SZ399005",8)==0 || memcmp(id,"SZ399006",8)==0)
			return 4;
		return -1;
	}

	//获取拼音首字母
	//支持gb2312,也支持大字符集的GBK编码
	//来自：http://blog.csdn.net/blackoto/article/details/4430983
	void GetMiniCode(char strName[], char result[])
	{
		static int li_SecPosValue[] = {
			1601, 1637, 1833, 2078, 2274, 2302, 2433, 2594, 2787, 3106, 3212,
			3472, 3635, 3722, 3730, 3858, 4027, 4086, 4390, 4558, 4684, 4925, 5249
		};

		static char* lc_FirstLetter[] = {
			"A", "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "O",
			"P", "Q", "R", "S", "T", "W", "X", "Y", "Z"
		};

		static char* ls_SecondSecTable =
			"CJWGNSPGCGNE[Y[BTYYZDXYKYGT[JNNJQMBSGZSCYJSYY[PGKBZGY[YWJKGKLJYWKPJQHY[W[DZLSGMRYPYWWCCKZNKYYGTTNJJNYKKZYTCJNMCYLQLYPYQFQRPZSLWBTGKJFYXJWZLTBNCXJJJJTXDTTSQZYCDXXHGCK[PHFFSS[YBGXLPPBYLL[HLXS[ZM[JHSOJNGHDZQYKLGJHSGQZHXQGKEZZWYSCSCJXYEYXADZPMDSSMZJZQJYZC[J[WQJBYZPXGZNZCPWHKXHQKMWFBPBYDTJZZKQHY"
			"LYGXFPTYJYYZPSZLFCHMQSHGMXXSXJ[[DCSBBQBEFSJYHXWGZKPYLQBGLDLCCTNMAYDDKSSNGYCSGXLYZAYBNPTSDKDYLHGYMYLCXPY[JNDQJWXQXFYYFJLEJPZRXCCQWQQSBNKYMGPLBMJRQCFLNYMYQMSQYRBCJTHZTQFRXQHXMJJCJLXQGJMSHZKBSWYEMYLTXFSYDSWLYCJQXSJNQBSCTYHBFTDCYZDJWYGHQFRXWCKQKXEBPTLPXJZSRMEBWHJLBJSLYYSMDXLCLQKXLHXJRZJMFQHXHWY"
			"WSBHTRXXGLHQHFNM[YKLDYXZPYLGG[MTCFPAJJZYLJTYANJGBJPLQGDZYQYAXBKYSECJSZNSLYZHSXLZCGHPXZHZNYTDSBCJKDLZAYFMYDLEBBGQYZKXGLDNDNYSKJSHDLYXBCGHXYPKDJMMZNGMMCLGWZSZXZJFZNMLZZTHCSYDBDLLSCDDNLKJYKJSYCJLKWHQASDKNHCSGANHDAASHTCPLCPQYBSDMPJLPZJOQLCDHJJYSPRCHN[NNLHLYYQYHWZPTCZGWWMZFFJQQQQYXACLBHKDJXDGMMY"
			"DJXZLLSYGXGKJRYWZWYCLZMSSJZLDBYD[FCXYHLXCHYZJQ[[QAGMNYXPFRKSSBJLYXYSYGLNSCMHZWWMNZJJLXXHCHSY[[TTXRYCYXBYHCSMXJSZNPWGPXXTAYBGAJCXLY[DCCWZOCWKCCSBNHCPDYZNFCYYTYCKXKYBSQKKYTQQXFCWCHCYKELZQBSQYJQCCLMTHSYWHMKTLKJLYCXWHEQQHTQH[PQ[QSCFYMNDMGBWHWLGSLLYSDLMLXPTHMJHWLJZYHZJXHTXJLHXRSWLWZJCBXMHZQXSDZP"
			"MGFCSGLSXYMJSHXPJXWMYQKSMYPLRTHBXFTPMHYXLCHLHLZYLXGSSSSTCLSLDCLRPBHZHXYYFHB[GDMYCNQQWLQHJJ[YWJZYEJJDHPBLQXTQKWHLCHQXAGTLXLJXMSL[HTZKZJECXJCJNMFBY[SFYWYBJZGNYSDZSQYRSLJPCLPWXSDWEJBJCBCNAYTWGMPAPCLYQPCLZXSBNMSGGFNZJJBZSFZYNDXHPLQKZCZWALSBCCJX[YZGWKYPSGXFZFCDKHJGXDLQFSGDSLQWZKXTMHSBGZMJZRGLYJB"
			"PMLMSXLZJQQHZYJCZYDJWBMYKLDDPMJEGXYHYLXHLQYQHKYCWCJMYYXNATJHYCCXZPCQLBZWWYTWBQCMLPMYRJCCCXFPZNZZLJPLXXYZTZLGDLDCKLYRZZGQTGJHHGJLJAXFGFJZSLCFDQZLCLGJDJCSNZLLJPJQDCCLCJXMYZFTSXGCGSBRZXJQQCTZHGYQTJQQLZXJYLYLBCYAMCSTYLPDJBYREGKLZYZHLYSZQLZNWCZCLLWJQJJJKDGJZOLBBZPPGLGHTGZXYGHZMYCNQSYCYHBHGXKAMTX"
			"YXNBSKYZZGJZLQJDFCJXDYGJQJJPMGWGJJJPKQSBGBMMCJSSCLPQPDXCDYYKY[CJDDYYGYWRHJRTGZNYQLDKLJSZZGZQZJGDYKSHPZMTLCPWNJAFYZDJCNMWESCYGLBTZCGMSSLLYXQSXSBSJSBBSGGHFJLYPMZJNLYYWDQSHZXTYYWHMZYHYWDBXBTLMSYYYFSXJC[DXXLHJHF[SXZQHFZMZCZTQCXZXRTTDJHNNYZQQMNQDMMG[YDXMJGDHCDYZBFFALLZTDLTFXMXQZDNGWQDBDCZJDXBZGS"
			"QQDDJCMBKZFFXMKDMDSYYSZCMLJDSYNSBRSKMKMPCKLGDBQTFZSWTFGGLYPLLJZHGJ[GYPZLTCSMCNBTJBQFKTHBYZGKPBBYMTDSSXTBNPDKLEYCJNYDDYKZDDHQHSDZSCTARLLTKZLGECLLKJLQJAQNBDKKGHPJTZQKSECSHALQFMMGJNLYJBBTMLYZXDCJPLDLPCQDHZYCBZSCZBZMSLJFLKRZJSNFRGJHXPDHYJYBZGDLQCSEZGXLBLGYXTWMABCHECMWYJYZLLJJYHLG[DJLSLYGKDZPZXJ"
			"YYZLWCXSZFGWYYDLYHCLJSCMBJHBLYZLYCBLYDPDQYSXQZBYTDKYXJY[CNRJMPDJGKLCLJBCTBJDDBBLBLCZQRPPXJCJLZCSHLTOLJNMDDDLNGKAQHQHJGYKHEZNMSHRP[QQJCHGMFPRXHJGDYCHGHLYRZQLCYQJNZSQTKQJYMSZSWLCFQQQXYFGGYPTQWLMCRNFKKFSYYLQBMQAMMMYXCTPSHCPTXXZZSMPHPSHMCLMLDQFYQXSZYYDYJZZHQPDSZGLSTJBCKBXYQZJSGPSXQZQZRQTBDKYXZK"
			"HHGFLBCSMDLDGDZDBLZYYCXNNCSYBZBFGLZZXSWMSCCMQNJQSBDQSJTXXMBLTXZCLZSHZCXRQJGJYLXZFJPHYMZQQYDFQJJLZZNZJCDGZYGCTXMZYSCTLKPHTXHTLBJXJLXSCDQXCBBTJFQZFSLTJBTKQBXXJJLJCHCZDBZJDCZJDCPRNPQCJPFCZLCLZXZDMXMPHJSGZGSZZQLYLWTJPFSYASMCJBTZKYCWMYTCSJJLJCQLWZMALBXYFBPNLSFHTGJWEJJXXGLLJSTGSHJQLZFKCGNNNSZFDEQ"
			"FHBSAQTGYLBXMMYGSZLDYDQMJJRGBJTKGDHGKBLQKBDMBYLXWCXYTTYBKMRTJZXQJBHLMHMJJZMQASLDCYXYQDLQCAFYWYXQHZ";

		int H, L, W;
		int result_no = 0;
		unsigned int i, stringlen = strlen(strName);
		int j;
		for (i = 0; i < stringlen; i++) {
			H = (unsigned char)(strName[i + 0]);
			L = (unsigned char)(strName[i + 1]);

			if (H < 0xA1 || L < 0xA1) {

				//不是汉字的忽略掉
				//result[result_no] = strName[i];
				//result_no++;
				continue;
			}
			else {
				W = (H - 160) * 100 + L - 160;
			}

			if (W > 1600 && W < 5590) {
				for (j = 22; j >= 0; j--) {
					if (W >= li_SecPosValue[j]) {
						result[result_no] = lc_FirstLetter[j][0];
						result_no++;
						i++;
						break;
					}
				}

				continue;
			}
			else {
				i++;
				W = (H - 160 - 56) * 94 + L - 161;
				if (W >= 0 && W <= 3007)
				{
					result[result_no] = ls_SecondSecTable[W];
					result_no++;
				}
				else {
					//注释掉，去掉全角的A、B、数字等
					//result[result_no]= (char)H;
					//++result_no;
					//result[result_no]= (char)L;
					//++result_no;
				}
			}
		}
		return;
	}


};


//所有中国股市的类型：
//忽略大小写 
//std::regex_constants::syntax_option_type fl = std::regex_constants::icase;
//// 保存查找的结果  
//std::smatch ms;
////A股表达式 
//std::regex Ashares("SH60[0-8]\\d{3}|SZ00[01256789]\\d{3}");        //A股
//std::regex SmallAndMedium("(SZ002\\d{3})");						//深圳中小板 
//std::regex Growth("(SZ300\\d{3})");								//深圳创业板 
//std::wregex Index(L"SH000\\d{3}|SH8[013]\\d{4}|SZ399\\d{3}");        //指数
//std::wregex Bshares(L"SH90\\d{4}|SZ20\\d{4}");					    //B股
//std::wregex Fund(L"SH5[01]\\d{4}|SZ184\\d{3}|SZ1[56]\\d{4}");        //基金
//std::wregex Bond(L"SH[012]\\d{5}|SZ1[0123]\\d{4}");                  //债券
//std::wregex Futures(L"SF\\w{6}");                                    //期货
//std::wregex Warrant(L"SH58\\d{4}|SZ03\\d{4}");					    //权证 
//if (std::regex_match(code, ms, Ashares))
//{
//	return ::Ashares;
//}
//if (std::regex_match(code, ms, SmallAndMedium))
//{
//	return ::SmallAndMedium;
//}
//if (std::regex_match(code, ms, Growth))
//{
//	return ::Growth;
//}

// if(std::regex_match(code,ms,Index))
//{
//	return ::Index;
//}
//if(std::regex_match(code,ms,Bshares))
//{
//	return ::Bshares;
//}
//	if(std::regex_match(code,ms,Fund))
//{
//	return ::Fund;
//}
//if(std::regex_match(code,ms,Bond))
//{
//	return ::Bond;
//}
//if(std::regex_match(code,ms,Warrant))
//{
//	return ::Warrant;
//}
//if(std::regex_match(code,ms,Futures))
//{
//	return ::Futures;
//}
#endif