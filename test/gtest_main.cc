#include <stdio.h>
#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from gtest_main.cc\n");
	testing::InitGoogleTest(&argc, argv);
	int result=RUN_ALL_TESTS();
	
	//如果命令行方式运行，则在此暂停，以便观察命令窗口
	//如果在测试资源管理器运行，则要注释掉，否则测试会一直运行并等待。
	//system("PAUSE");
	
	return result;
}