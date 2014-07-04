#include <string>
//#include <locale>
#include <codecvt>
#include "gtest/gtest.h"
using namespace std;



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
	const wchar_t wcharString[] = L"a"; //保存0x61,0x00,0x00，
	const wchar_t nextString[] = L"啊";
	EXPECT_EQ((size_t)1, strlen(((char *)wcharString)));
	EXPECT_EQ(0x61, ((char *)wcharString)[0]);
	EXPECT_EQ(0x00, ((char *)wcharString)[1]);
	EXPECT_EQ(0x00, ((char *)wcharString)[2]);
	EXPECT_EQ((size_t)2, strlen(((char *)nextString)));
	EXPECT_EQ((size_t)1, wcslen(nextString));//作为宽字符算1位

	EXPECT_STREQ("1\\", R"(1\)");//Vs2013中R表示原生字符串
	//这是不能编译的：Assert::AreEqual("1\\", R"1\");
}