#include <fstream>
#include "sqlite/sqlite3.h"

#include "global.h"
#include "db.h"

//注意：权息数据从新到旧顺序排列
class DbExright
{
public:
	
	//读取...
	//当返回的vector长度为0,则不合法

	//insert使用vector的begin end迭代器

	bool check(const wchar_t *filename)
	{
		bool isValidFile = false;
		const long ExrightFlag =0xFF43C832;//注意，文件中的4个字节的顺序为0x32C843FF，顺序正好相反。
		size_t size=0;
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
			func(L"代码表没有安装，此次安装除权表失败!",100);
			return 0;
		}
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
		//stock_list.clear();
		ExRight exRight;
		char *buffer = ptr.get();		
		//从第12个字节开始
		int position = 12; //第8位开始是FFFFFFFF标记，然后是第一只股票的id
		int flag = -1;
		int order = -1; //代码在全局代码表中的顺序
		char id[9] = { 0 }; //保存文件中的id

		int id_number = 0;

		Db conn;
		conn.Begin();
		auto cmd = conn.CreateQuery("UPDATE STOCK SET EXRIGHTS=? WHERE ID=?");
		while (position<fileSize)
		{
			++id_number;
			//读取代码
			memcpy(id, (char *)buffer + position, 8); //将8位加入id
			//stock.Id[8] = 0;
			order = g_stock.FindStock(id);
			if (order>0)
			{
				g_stock.Data[order].ExRightSet.clear();
			}
			position += 16;
			flag = *((long *)(buffer + position));
			while (flag > 0)
			{
				if (order > 0)
				{
					exRight.QuoteTime = flag; //日期
					exRight.AddStock = *((float *)(buffer + position + 4));  //送股
					exRight.BuyStock = *((float *)(buffer + position + 8));  //配股
					exRight.Price = *((float *)(buffer + position + 12));    //配股价格
					exRight.Bonus = *((float *)(buffer + position + 16));    //分红
					g_stock.Data[order].ExRightSet.push_back(exRight);
				}
				position += 20;
				if ((position < fileSize))
				{
					flag = *((long *)(buffer + position));
					if (flag <= 0)
						position += 4;
				}
				else
					flag = -1;
			}
			//此时更新数据库
			if (order>0)
			{
				cmd.Bind(1,g_stock.Data[order].ExRightSet, id);
				cmd.ExcuteNonQuery();
			}
			if ((id_number % period) == 0)
			{
				func(nullptr, id_number * 80 / g_stock.Data.size());
			}
		}
		
		conn.Commit();
		//func(nullptr, 100);
		return id_number;
		//写入所有权息数据
	}
	
};