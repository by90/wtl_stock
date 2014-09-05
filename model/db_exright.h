#include <fstream>
#include "sqlite/sqlite3.h"

#include "global.h"
#include "db.h"


class DbExright
{
public:
	
	//读取...
	//当返回的vector长度为0,则不合法

	//insert使用vector的begin end迭代器

	

	size_t bulk_insert(const wchar_t *_file, int totals = -1, int period = 2000, std::function<void(const wchar_t *, int)> func = nullptr)
	{
		std::vector<Stock> stock_list;

		//读取文件
		std::ifstream reader;  //注意，来自<fstream>而非<iostream>
		reader.open(_file,std::ios::binary |std::ios::in);  //以二进制只读方式打开文件
		reader.seekg(0, std::ios::end);//以文件尾定位,beg以文件首字节为0开始定位，cur以当前位置开始定位
		long fileSize = (long)reader.tellg();//得到文件大小
		reader.seekg(0, std::ios::beg); //回到文件第一个字节
		std::shared_ptr<char> ptr(new char[fileSize], [](char* ptr){delete[] ptr; });
		reader.read(ptr.get(), fileSize); //将全部文件读取到内存
		reader.close(); //关闭文件

		//将Exrights读取到vector
		stock_list.clear();
		Stock stock;
		ExRight exRight;
		char *buffer = ptr.get();		
		//从第12个字节开始
		int position = 12;
		int flag = -1;
		string id = "";
		while (position<fileSize)
		{
			//读取代码
			memcpy(stock.Id,(char *)buffer + position, 8); //将8位加入id
			stock.Id[8] = 0;

			stock.ExRightSet.clear();
			position += 16;
			flag = *((long *)(buffer + position));
			while (flag>0)
			{
				exRight.QuoteTime = flag; //日期
				exRight.AddStock = *((float *)(buffer + position + 4));  //送股
				exRight.BuyStock = *((float *)(buffer + position + 8));  //配股
				exRight.Price = *((float *)(buffer + position + 12));    //配股价格
				exRight.Bonus = *((float *)(buffer + position + 16));    //分红
				stock.ExRightSet.push_back(exRight);
				position += 20;
				if ((position<fileSize))
				{
					flag = *((long *)(buffer + position));
					if (flag <= 0)
						position += 4;
				}
				else
					flag = -1;
			}

			stock_list.push_back(stock);
		}

		//删除所有数据库中的权息数据

		//写入所有权息数据
	}
};