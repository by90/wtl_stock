//头文件数据库访问类
#include "global.h"
#include "sqlite/sqlite3.h"
#include "db.h"
#include "dad_parse_iterator.h"
#include <vector>
#include <algorithm>

#ifndef db_code_h
#define db_code_h

class DbCode
{
public:
	DbCode() = default;
	~DbCode() = default;

	//获取全部股票代码信息，保存在global::StockSet中
	//时机可在应用启动时，而每只股票相关的日线、权息数据，数据量较大，根据情形按需载入
	//应用初启动的时候，Global::init调用get_date_range获得已安装数据的起止日期，并调用此方法获得代码表
	//安装日线数据时，可考虑凡事大于StockSet中最后日期者，按顺序重新读入
	//读取全部日线，需要2000-3000次的载入操作，每次约2000条数据，大致为600万次操作。
	static void GetAllCode(bool refresh=false)
	{
		if (DbCode::get_stock_list().size()>0)
		{
			if (!refresh)	return;
			else
				DbCode::get_stock_list().clear();
		}
		Db connection_;
		auto query = connection_.create_query(L"select * from Stock order by Id");
		Stock stock;
		int market, catalog;
		while (query.Execute(stock.Id, market,catalog, stock.Title, stock.MiniCode))
		{
			stock.Market = (MarketEnum)market;
			stock.Catalog = (CatalogEnum)catalog;
			DbCode::get_stock_list().push_back(stock);
		}
		
	}

	static vector<Stock> &get_stock_list()
	{
		static vector<Stock> stock_list;
			return stock_list;
	}

	//如返回值result小于0，表示没有找到，-1*result-1为插入位置
	static int FindStock(char id[])
	{
		Stock stock;
		strcpy_s(stock.Id, id); //拷贝
		pair<vector<Stock>::iterator, vector<Stock>::iterator> bounds;
		bounds = std::equal_range(DbCode::get_stock_list().begin(), DbCode::get_stock_list().end(), stock);//返回
		if (bounds.first == bounds.second)
			return -1 - std::distance(DbCode::get_stock_list().begin(), bounds.first); //没有找到,返回负值，表示插入的位置，注意，若begin为0，则返回-1
		return (std::distance(DbCode::get_stock_list().begin(), bounds.first)); //返回找到的第一个与第一个的距离，也就是序号
	}

	//根据代码获取交易所
	static MarketEnum GetMarket(const char id[])
	{
		return (id[1] == 'H') ? ::ShangHai : (id[1] == 'Z') ? ::ShenZhen : (::QiHuo);
	}

	//根据代码，获取股票类型，"其它"类型将被忽略
	//#include <regex> 正则表达式，效率可能略低
	static CatalogEnum GetCatalog(const char id[])
	{
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

		//只要这四个指数，
		if (id == "SH000001" || id == "SZ399001" || id == "SZ399005" || id == "SZ399006")
			return ::Index;

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

		//判断上证
		if (memcmp("SH6", id, 3) == 0)
			return ::Ashares;
		
		//先判断深圳中小板
		if (memcmp("SZ002", id, 5) == 0)
			return ::SmallAndMedium;

		//再判断深圳创业板
		if (memcmp("SZ300", id, 5) == 0)
			return ::Growth;

		//然后深圳A股
		if (memcmp("SZ00", id, 4) == 0)
			return ::Ashares;

		//四个指数
		if (id == "SH000001" || id == "SZ399001" || id == "SZ399005" || id == "SZ399006")
			return ::Index;
		return ::Others;
	}

	//获取拼音首字母
	//支持gb2312,也支持大字符集的GBK编码
	//来自：http://blog.csdn.net/blackoto/article/details/4430983
	static void GetMiniCode(char strName[],char result[])
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

#endif