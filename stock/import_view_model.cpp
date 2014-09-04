#include "stdafx.h"
#include "import_model_base.h"
#include "import_view.h"
#include "import_view_model.h"


ImportViewModel::ImportViewModel(ImportView *_view)
{
	view_ = _view;
}

void ImportViewModel::set_installed_info(unsigned long _begin, unsigned long _end) //初始化，获取已经安装数据
{
	if (_begin == 0 || _end == 0)
		installed_info_ = L"已经安装：没有数据";
	else
	{
		installed_info_.clear();
		installed_info_ = L"已经安装：";
		stdmore::time_to_wstring((time_t)_begin, L"%Y-%m-%d", installed_info_);
		installed_info_ += L"到";
		stdmore::time_to_wstring((time_t)_end, L"%Y-%m-%d", installed_info_);
	}
}
//初始化，由View的InitDialog调用
//将import_catalog设为0，同时建立model对象
void ImportViewModel::Init()
{
	switch (this->import_catalog_)
	{
	case 0:
		model_.reset(new ImportModelQuote);
		break;
	case 1:
		model_.reset(new ImportModelQuote);
		break;
	default:
		model_.reset(new ImportModelQuote);
		break;
	}
	unsigned long start_date = 0;
	unsigned long end_date = 0;
	model_->GetSavedDate(start_date,end_date);
	set_installed_info(start_date, end_date);//获取"已经安装数据"
	view_->DoDataExchange(false, IDC_STATIC_SAVED); //从数据到UI,false表示是否获取ui数据

	set_ui_state(0);//状态设为0
}

//选择文件，并检查合法性
void ImportViewModel::SelectFile()
{

}
//安装，并使用回调更新进度文本控件和进度条
void ImportViewModel::ImportFile(std::function<void(const char *, int)> func)
{

}

//改变导入文件的类型，将状态设为0
void ImportViewModel::ChangeImportFileType()
{

}

//改变View的状态，由此设定各控件的可用性和可见性
void ImportViewModel::set_ui_state(int _ui_state) 
{
	ui_state_ = _ui_state;
}

//删除全部日线和代码表
void ImportViewModel::RemoveInstalledQuote()
{

}