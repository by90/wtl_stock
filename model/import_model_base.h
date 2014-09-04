#include <functional>

#ifndef STOCK_MODEL_IMPORT_MODEL_BASE_H
#define STOCK_MODEL_IMPORT_MODEL_BASE_H

//导入数据的model类基类
//纯虚接口类
class ImportModelBase
{
public:
	virtual void Init(); //初始化，获取已经安装数据
	virtual bool CheckSourceFile(const wchar_t *_file); //检查文件是否合法
	virtual void ImportFile(std::function<void(const char *, int)> func);
};

class ImportModelQuote:public ImportModelBase
{
public:
	virtual void Init() //初始化，获取已经安装数据
	{

	}
	virtual bool CheckSourceFile(const wchar_t *_file) //检查文件是否合法
	{

	}
	virtual void ImportFile(std::function<void(const char *, int)> func)
	{

	}

};

class ImportModelExright :public ImportModelBase
{
public:
	virtual void Init() //初始化，获取已经安装数据
	{

	}
	virtual bool CheckSourceFile(const wchar_t *_file) //检查文件是否合法
	{

	}
	virtual void ImportFile(std::function<void(const char *, int)> func)
	{

	}

};

#endif