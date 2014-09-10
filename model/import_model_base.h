#include <functional>
#include <sstream>
#include "global.h"
#include "dad_file_parse.h"
#include "db_quote.h"
#include "timer.h"
#include "stdmore.h"
#ifndef STOCK_MODEL_IMPORT_MODEL_BASE_H
#define STOCK_MODEL_IMPORT_MODEL_BASE_H

//导入数据的model类基类
//纯虚接口类
class ImportModelBase
{
public:
	virtual void GetSavedInfo(wstring &_saved,bool _fetch=false)=0; //初始化，获取已经安装数据
	virtual void UpdateSavedInfo(wstring &_saved) = 0; //已经安装数据变化
	virtual bool CheckSourceFile(const wchar_t *_file, wstring &_selected) = 0; //检查文件是否合法,返回文件的说明
	virtual void ImportFile(const wchar_t *_file, std::function<void(const wchar_t *, int)> func) = 0;
};

class ImportModelQuote:public ImportModelBase
{
public:
	DbQuote quote_;
	dad_file_parse parser_;

	virtual void GetSavedInfo(wstring &_saved, bool _fetch = false)
	{
		if (_fetch)
		  quote_.GetSavedDate(g_stock.BeginDate, g_stock.EndDate);
		if (g_stock.BeginDate == 0 || g_stock.BeginDate == 0)
			_saved = L"已经安装：没有数据";
		else
		{
			_saved.clear();
			_saved= L"已经安装：";
			stdmore::time_to_wstring((time_t)g_stock.BeginDate, L"%Y-%m-%d", _saved);
			_saved += L"到";
			stdmore::time_to_wstring((time_t)g_stock.EndDate, L"%Y-%m-%d", _saved);
		}
	}

	virtual void UpdateSavedInfo(wstring &_saved)
	{
		if (g_stock.BeginDate == 0)
		{
			if (parser_.m_start_date != 0)
				g_stock.BeginDate = parser_.m_start_date;
		}
		else
		{
			if (parser_.m_start_date != 0 && parser_.m_start_date< g_stock.BeginDate)
				g_stock.BeginDate = parser_.m_start_date;
		}

		if (g_stock.EndDate == 0)
		{
			if (parser_.m_end_date != 0)
				g_stock.EndDate = parser_.m_end_date;
		}
		else
		{
			if (parser_.m_end_date != 0 && parser_.m_end_date> g_stock.EndDate)
				g_stock.EndDate = parser_.m_end_date;
		}
		GetSavedInfo(_saved);
	}

	virtual bool CheckSourceFile(const wchar_t *_file, wstring &_selected) //检查文件是否合法
	{
		if (parser_.check(_file))
		{
			_selected = L" ";
			_selected = L"准备安装：";
			stdmore::time_to_wstring((time_t)parser_.m_start_date, L"%Y-%m-%d", _selected);
			_selected += L"到";
			stdmore::time_to_wstring((time_t)parser_.m_end_date, L"%Y-%m-%d", _selected);
			return true;
		}
		_selected = L"准备安装：您选的文件格式不对";
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
	virtual void GetSavedInfo(wstring &_saved, bool _fetch = false)
	{
		_saved = L"";
	}

	virtual void UpdateSavedInfo(wstring &_saved)
	{
		_saved = L"";

	}

	virtual bool CheckSourceFile(const wchar_t *_file, wstring &_selected) //检查文件是否合法
	{
		//if (parser_.check(_file))
		//{
		//	_selected = L" ";
		//	_selected = L"准备安装：";
		//	stdmore::time_to_wstring((time_t)parser_.m_start_date, L"%Y-%m-%d", _selected);
		//	_selected += L"到";
		//	stdmore::time_to_wstring((time_t)parser_.m_end_date, L"%Y-%m-%d", _selected);
		//	return true;
		//}
		//_selected = L"准备安装：您选的文件格式不对";
		//return false;
		return true;
	}
	virtual void ImportFile(const wchar_t *_file, std::function<void(const wchar_t *, int)> func)
	{
		wostringstream ss;
		timer time_used; //开始计时
		time_used.reset();
		ss << L"正在安装...";
		//parser_.open(_file);

		//wostringstream ss;
		//ss << L"共" << parser_.m_quote_count << L"条记录，正在安装...";
		//func(ss.str().c_str(), 0);

		//int inserted = quote_.bulk_insert(parser_.begin(), parser_.end(), parser_.m_quote_count, 2000, func);
		//parser_.close(); //这个函数后台执行，函数内顺序执行，因此这里关闭可行

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
};

#endif