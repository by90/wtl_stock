#include <Windows.h>
#include "gtest/gtest.h"

class uiTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{

	}
};

//
//rcArea，整个区域
//left,top：左上角坐标，大于0为固定数值，-1到-100表示按rcArea宽度和高度的百分比。
//width,height：宽度和高度，大于0为固定数值，等于0则维持不变
//函数先计算大小，再确定左上角的位置
void CalcRect(RECT rcArea,RECT rcRate,RECT &rcChild, bool mode = FALSE)
{
	//初始化：宽度和高度
	int widthArea = rcArea.right - rcArea.left + 1;
	int heightArea = rcArea.bottom - rcArea.top + 1;
	int width = rcChild.right - rcChild.left + 1;
	int height = rcChild.bottom - rcChild.top + 1;

	//首先计算大小
	//mode仅仅对左上角坐标有意义
	//rcRate.right=0,则宽度维持原状
	//rcRate.right>0,则宽度设为rcRate的right值
	//rcRate.right<0，则根据模式，设定为rcArea宽度为基础、或宽度差为基础的相对值，-1代表1%，-100为100%
	if (rcRate.right > 0) width = rcRate.right;
	if (rcRate.right < 0) width = -widthArea*rcRate.right / 100;

	//rcRate.right=0,则高度维持原状
	//rcRate.bottom>0,则高度设为rcRate的bottom值
	//rcRate.bottom<0，则根据模式，设定为rcAre高度为rcArea高度的相对值，-1代表1%，-100为100%
	if (rcRate.bottom > 0) height = rcRate.bottom;
	if (rcRate.bottom < 0) height = -heightArea*rcRate.bottom/ 100;
	
	
	int widthBase = mode?(widthArea - width):widthArea;
	int heightBase = mode ? (heightArea - height) : heightArea;

	//然后确定位置
	if (rcRate.left >= 0)
		rcChild.left = rcArea.left+rcRate.left; //rcRate是相对父RECT的值，我们需要得到窗口坐标
	else
		rcChild.left =rcArea.left-widthBase*rcRate.left/100;//这里算坐标，不应减去1
	
	//这里是rcArea为正数，始终会加正数，因此不可能小于0
	//if (rcChild.left < 0) rcChild.left = 0;

	if (rcRate.top >= 0)
		rcChild.top =rcArea.top+rcRate.top;
	else
		rcChild.top =rcArea.top-heightBase*rcRate.top / 100;

	//至此，位置已经正确，此时修改右下坐标
	rcChild.right = rcChild.left + width - 1;
	rcChild.bottom = rcChild.top + height - 1;
}

////db类的静态函数set_default
TEST_F(uiTest, CalcRect_Test)
{
	//最终得到的坐标，是针对rcArea的父窗口
	RECT rcParent = { 23,33, 424, 536};//这是相对父窗口的坐标
	RECT rcRate = {0,0,0,0};//这是比例
	RECT rcNow = { 0, 0, 254, 156 }; //一般获取客户端大小即可(有边框需使用WindowRect,无所谓，大小才是重要的)

	//rcRate设为全0，位置为rcParent的左上角坐标，大小保持不变
	RECT rcTarget = rcNow;//用于测试
	CalcRect(rcParent, rcRate, rcTarget);
	EXPECT_EQ(23, rcTarget.left);
	EXPECT_EQ(33, rcTarget.top);
	EXPECT_EQ(23+255-1,rcTarget.right); //子窗口宽度255
	EXPECT_EQ(33+157-1, rcTarget.bottom); //子窗口高度157


	//大小设为固定值
	rcRate = {0,0,30,100};
	rcTarget = rcNow;
	CalcRect(rcParent, rcRate, rcTarget);
	EXPECT_EQ(23+30-1, rcTarget.right);
	EXPECT_EQ(33+100-1, rcTarget.bottom);

	//大小设为相对值
	rcRate = { 0, 0, -10, -20 };
	rcTarget = rcNow;
	CalcRect(rcParent, rcRate, rcTarget);
	EXPECT_EQ(23 + 40-1, rcTarget.right); //宽度402，10%为40.2
	EXPECT_EQ(33 + 100-1, rcTarget.bottom); //高度504，20%为100.8

	//位置设为相对值：
	rcRate = { -10, -10, 30, 100 };
	rcTarget = rcNow;
	CalcRect(rcParent, rcRate, rcTarget);
	EXPECT_EQ(23 + 40 , rcTarget.left);//宽度402，10%为40
	EXPECT_EQ(33 + 50 , rcTarget.top); //高度504,10%为50

	//位置和大小都设为相对值：
	rcRate = { -10, -10, -10, -20 };
	rcTarget = rcNow;
	CalcRect(rcParent, rcRate, rcTarget);
	EXPECT_EQ(23 + 40, rcTarget.left);//宽度402，10%为40
	EXPECT_EQ(33 + 50, rcTarget.top); //高度504,10%为50
	EXPECT_EQ(23 + 40+40-1, rcTarget.right);//宽度402，10%为40
	EXPECT_EQ(33 + 50+100-1, rcTarget.bottom); //高度504,20%为100.8



}
