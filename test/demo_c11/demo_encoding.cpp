#include <string>
//#include <locale>
#include <codecvt>
#include "gtest/gtest.h"
using namespace std;

//结论：
//我们会使用std::string形式的gb2312，是因为数据源用这个表示汉字
//程序中全体使用unicode
//sqlite中，unicode的查询若有问题，则只好使用utf8，这一点要测试一下。
//换言之，除了外部数据源需要，我们在整个应用中统一使用unicode或utf8


//将wstring转换成utf8，使用c++标准方式，不使用windows api
std::string to_utf8(const std::wstring& str)
{
	//定义一个转换器
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	//将wstring转换成wtf8
	return conv.to_bytes(str);
	
	//如果反过来转换：
	// std::wstring wideStr = conv.from_bytes(narrowStr);
}

//gb2312转unicode
std::wstring gb2312_to_unicode(const std::string& str)
{
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>conv(new std::codecvt<wchar_t, char, std::mbstate_t>("CHS"));//GBK<->Unicode转换器
	//定义一个转换器
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	//将wstring转换成wtf8
	return conv.from_bytes(str);

	//如果反过来转换：
	// std::wstring wideStr = conv.from_bytes(narrowStr);
}

//编码转换
TEST(demo, encoding_convert)
{
	auto result = to_utf8(L"你好");
	EXPECT_EQ(6, result.length());
}

//c++ 11新字符类型：char16_t,char_32t,分别使用u和U，Vs2013不支持
//utf_16分成utf16_LE和utf16_BE两种，前者是小头，比较常用，为0x61,0x00，cpu是倒着吃的，所以能够提高速度
//L"a"，实际上是小头，所以vs2013默认的是0x61,0x00这种即utf_16BE...记住Big
//utf16_BE,big endian，表达为0x00,0x61
TEST(demo, encoding)
{
	const wchar_t wcharString[] = L"a"; //保存0x61,0x00,0x00、0x00，
	const wchar_t nextString[] = L"啊";
	const char thirdString[] = "啊";

	int size = sizeof(L"ab");
	//对于wchar数组
	EXPECT_EQ((size_t)1, strlen(((char *)wcharString))); //字符串角度，只有一个字符
	EXPECT_EQ((size_t)4, sizeof(wcharString)); //长度为4个字节

	//注意unsigned char *才能和数字正确比较
	EXPECT_EQ(0x61, ((unsigned char *)wcharString)[0]); //第一个字节为0x61
	EXPECT_EQ(0x00, ((unsigned char *)wcharString)[1]); //第二个为0，由于该字符第二个字节为0，所以char *的len为1
	EXPECT_EQ(0x00, ((unsigned char *)wcharString)[2]); //结尾符号为两个0
	EXPECT_EQ(0x00, ((unsigned char *)wcharString)[3]);

	//汉字：
	EXPECT_EQ((size_t)2, strlen(((char *)nextString))); //作为字符串，是两位
	EXPECT_EQ((size_t)1, wcslen(nextString));//作为宽字符是1位
	EXPECT_EQ((size_t)4, sizeof(nextString)); //实际长度为4

	EXPECT_EQ(0x4A, ((unsigned char *)nextString)[0]); //'啊'的unicode编码第一位
	EXPECT_EQ(0x55, ((unsigned char *)nextString)[1]); //'啊'的unicode编码第二位
	EXPECT_EQ(0x00, ((unsigned char *)nextString)[2]); //剩下两位0为结束符号
	EXPECT_EQ(0x00, ((unsigned char *)nextString)[3]);
    
	//char[]表达的汉字
	EXPECT_EQ((size_t)3, sizeof(thirdString)); //长度应是3位，一个双字节字符和0
	EXPECT_EQ(0xB0, ((unsigned char *)thirdString)[0]); //'啊'的unicode编码第一位
	EXPECT_EQ(0xA1, ((unsigned char *)thirdString)[1]); //'啊'的unicode编码第二位
	EXPECT_EQ(0x00, ((unsigned char *)thirdString)[2]); //剩下两位0为结束符号

	std::wstring result = gb2312_to_unicode(thirdString);
	auto p = result.c_str();
	EXPECT_EQ(0x4A, ((unsigned char *)p)[0]); //'啊'的unicode编码第一位
	EXPECT_EQ(0x55, ((unsigned char *)p)[1]); //'啊'的unicode编码第二位
	EXPECT_EQ(0x00, ((unsigned char *)p)[2]); //剩下两位0为结束符号
	EXPECT_EQ(0x00, ((unsigned char *)p)[3]);

	//结论：char数组的汉字，和wchar_t数组的汉字，其字节是不同的
	//1.同一个汉字，char数组为3位，前两位表示一个汉字，最后是0，wchar_t则是四位，后两位为0
	//2.同样的"啊",wchar_t是unicode，为4a 55,char为gb2312，为b0,a1
	//3.因此char[]到wchar_t需要转换成unicode

	//
	//EXPECT_STREQ("1\\", R"(1\)");//Vs2013中R表示原生字符串
	//这是不能编译的：Assert::AreEqual("1\\", R"1\");
}