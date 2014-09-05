#include <functional>
#include <sstream>
#include "global.h"
#include "dad_file_parse.h"
#include "db_quote.h"
#include "timer.h"
#ifndef STOCK_MODEL_IMPORT_MODEL_BASE_H
#define STOCK_MODEL_IMPORT_MODEL_BASE_H

//导入数据的model类基类
//纯虚接口类
class ImportModelBase
{
public:
	virtual void GetSavedDate(unsigned long &_start, unsigned long &_end)=0; //初始化，获取已经安装数据
	virtual bool CheckSourceFile(const wchar_t *_file, unsigned long &_start, unsigned long &_end, unsigned long &_count) = 0; //检查文件是否合法
	virtual void ImportFile(const wchar_t *_file, std::function<void(const wchar_t *, int)> func) = 0;
};

class ImportModelQuote:public ImportModelBase
{
public:
	DbQuote quote_;
	dad_file_parse parser_;

	virtual void GetSavedDate(unsigned long &_start, unsigned long &_end)
	{
		quote_.GetSavedDate(global::begin_date, global::end_date);
		_start = global::begin_date;
		_end = global::end_date;
	}
	virtual bool CheckSourceFile(const wchar_t *_file, unsigned long &_start, unsigned long &_end, unsigned long &_count) //检查文件是否合法
	{
		_start = 0;
		_end = 0;
		if (parser_.check(_file))
		{
			_start = parser_.m_start_date;
			_end = parser_.m_end_date;
			_count = parser_.m_quote_count;
			return true;
		}
		return false;
	}
	virtual void ImportFile(const wchar_t *_file,std::function<void(const wchar_t *, int)> func)
	{
		timer time_used; //开始计时
		time_used.reset();

		parser_.open(_file);

		wostringstream ss;
		ss << L"共" << parser_.m_quote_count << L"条记录，正在安装...";
		func(ss.str().c_str(), 0); 

		int inserted=quote_.bulk_insert(parser_.begin(), parser_.end(),parser_.m_quote_count, 2000, func);
		parser_.close(); //这个函数后台执行，函数内顺序执行，因此这里关闭可行

		ss.clear();
		ss << L"完成,耗时";
		auto used = time_used.elapsed_seconds();
		if (used <= 0)
			ss << L"不足1秒!";
		else
			if (used >= 60)
				ss << used / 60 << L"分" << used % 60 << L"秒!";
			else
				ss << used << L"秒!";
		func(ss.str().c_str(), 100);
	}
private:
	//根据日期获得时间字符串
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