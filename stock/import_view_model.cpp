#include "stdafx.h"
#include "import_model_base.h"
#include "import_view.h"
#include "import_view_model.h"
#include "ui_state.h"


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

void ImportViewModel::CheckFile()
{
	wchar_t temp[MAX_PATH] = { 0 };
	view_->GetDlgItemText(IDC_EDIT_PATH, temp, MAX_PATH);
	if (temp != selected_file_)  //对话框选择文件，预先已经令两者一致，只有编辑框才可能调用
	{
		selected_file_.clear();
		selected_file_.append(temp);

		unsigned long start_date, end_date;
		if (this->model_->CheckSourceFile(selected_file_.c_str(), start_date, end_date,file_count_))
		{
			set_ui_state(1);
			progress_info_ = L" ";
			selected_info_ = L"准备安装：";
			stdmore::time_to_wstring((time_t)start_date, L"%Y-%m-%d", selected_info_);
			selected_info_ += L"到";
			stdmore::time_to_wstring((time_t)end_date, L"%Y-%m-%d", selected_info_);
		}
		else
		{
			set_ui_state(0);
			selected_info_ = L"准备安装：您选的文件格式不对";
		}

		view_->DoDataExchange(FALSE, IDC_STATIC_OPENED);
		view_->DoDataExchange(FALSE, IDC_STATIC_INFO);
	}
}
//选择文件，并检查合法性
void ImportViewModel::SelectFile()
{
	std::wstring sDest;
	LPCTSTR pfileName = selected_file_.empty() ? NULL : selected_file_.c_str();
	CFileDialog dlg(TRUE, NULL, pfileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("所有文件 (*.*)\0*.*\0Dad文件 (*.Dad)\0*.exe\0Dat文件 (*.dat)\0\0"),view_->m_hWnd);
	if (dlg.DoModal() == IDOK)
	{
		wchar_t temp[MAX_PATH] = { 0 };
		view_->GetDlgItemText(IDC_EDIT_PATH, temp, MAX_PATH); //取得编辑框的路径
		if (temp != dlg.m_szFileName)
			view_->SetDlgItemTextW(IDC_EDIT_PATH, dlg.m_szFileName);
		CheckFile();
	}
}

void ImportViewModel::ImportFile(std::function<void(const char *, int)> func)
{
	model_->ImportFile(selected_file_.c_str(),func);
}
//安装，并使用回调更新进度文本控件和进度条
void ImportViewModel::RunImportFile()
{
	set_ui_state(2); //正在运行，调整界面

	wostringstream info;
	info << L"共" << file_count_ << L"条记录，正在安装...";
	progress_info_.clear();
	progress_info_.append(info.str());
	view_->SetDlgItemTextW(IDC_STATIC_INFO, progress_info_.c_str());

	timer time_used;
	time_used.reset();

	std::thread *t;
	t = new thread(&ImportViewModel::ImportFile,this,[this, t, time_used](const char *err, int now){
		if (err)
		{
			MessageBoxA(view_->m_hWnd, err, "导入过程出错", 0);
		}
		else if (::IsWindow(view_->m_progressBar.m_hWnd))
		{
			view_->m_progressBar.SetPos(now * 100 / file_count_ + 1);
			if (now == file_count_)
			{

				wstringstream ss;
				ss << L"完成,耗时";
				auto used = time_used.elapsed_seconds();
				if (used <= 0)
					ss << L"不足1秒!";
				else
					if (used >= 60)
						ss << used / 60 << L"分" << used % 60 << L"秒!";
					else
						ss << used << L"秒!";
				progress_info_.append(ss.str());

				//此处重新计算
				//model.set_saved_string();

	
				set_ui_state(3); //完成，调整界面
				view_->SetDlgItemTextW(IDC_STATIC_SAVED, installed_info_.c_str());
				view_->SetDlgItemTextW(IDC_STATIC_INFO, progress_info_.c_str());
				
				//model.parser.close(); //安装完毕后，清除缓存的Dad文件
				ThreadTask::Remove(import_id_);
			}
		}
	});
	import_id_ = t->get_id();
	ThreadTask::Add(import_id_, L"导入行情");
	t->detach(); //从主线程分离后执行
}

//改变导入文件的类型，将状态设为0
void ImportViewModel::ChangeImportFileType()
{

}

//改变View的状态，由此设定各控件的可用性和可见性
void ImportViewModel::set_ui_state(int _ui_state) 
{
	if (ui_state_ == _ui_state && ui_state_!=0)
		return;
	ui_state_ = _ui_state;
	switch (_ui_state)
	{
	case 0:
		::SetEnable(view_, false, IDC_BUTTON_INSTALL);
		::SetEnable(view_, true, IDC_BUTTON_SELECT, IDC_EDIT_PATH, IDC_BUTTON_REMOVE, IDC_RADIO_QUOTE, IDC_RADIO_EXRIGHT, IDC_RADIO_BASE, IDC_RADIO_CATALOG);

		::SetVisble((CWindow *)view_, false, IDC_PROGRESS_IMPORT);
		break;
	case 1:
		::SetEnable(view_, true, IDC_BUTTON_INSTALL, IDC_BUTTON_SELECT, IDC_EDIT_PATH, IDC_BUTTON_REMOVE, IDC_RADIO_QUOTE, IDC_RADIO_EXRIGHT, IDC_RADIO_BASE, IDC_RADIO_CATALOG);
		::SetVisble((CWindow *)view_, false, IDC_PROGRESS_IMPORT);
		break;
	case 2:
		::SetEnable(view_, false, IDC_BUTTON_INSTALL,IDC_BUTTON_SELECT, IDC_EDIT_PATH, IDC_BUTTON_REMOVE, IDC_RADIO_QUOTE, IDC_RADIO_EXRIGHT, IDC_RADIO_BASE, IDC_RADIO_CATALOG);
		::SetVisble((CWindow *)view_, true, IDC_PROGRESS_IMPORT);
		break;
	case 3:
		::SetEnable(view_, true, IDC_BUTTON_SELECT, IDC_EDIT_PATH, IDC_BUTTON_REMOVE, IDC_RADIO_QUOTE, IDC_RADIO_EXRIGHT, IDC_RADIO_BASE, IDC_RADIO_CATALOG);
		break;
	}
}

//删除全部日线和代码表
void ImportViewModel::RemoveInstalledQuote()
{

}