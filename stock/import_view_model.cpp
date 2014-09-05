#include "stdafx.h"
#include "import_model_base.h"
#include "import_view.h"
#include "import_view_model.h"
#include "ui_state.h"

ImportViewModel::ImportViewModel(ImportView *_view)
{
	view_ = _view;
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

	model_->GetSavedInfo(installed_info_,true);
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
		if (this->model_->CheckSourceFile(selected_file_.c_str(), selected_info_))
		{
			set_ui_state(1);
			progress_info_ = L" ";
		}
		else
		{
			set_ui_state(0);
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

//安装，并使用回调更新进度文本控件和进度条
void ImportViewModel::RunImportFile()
{
	set_ui_state(2); //正在运行，调整界面

	timer time_used;
	time_used.reset();

	std::thread *t;
	//thread的参数
	//1.&ImportModelBase::ImportFile 表示哪个类的哪个函数
	//2.model_.get() 表示指向哪个变量的指针，第一个参数所指类的实例
	//3和4，为函数提供的参数
	t = new thread(&ImportModelBase::ImportFile,model_.get(),
		selected_file_.c_str(),
		[this, t, time_used](const wchar_t *info, int now){
		if (info)
		{
			view_->SetDlgItemTextW(IDC_STATIC_INFO, info);
		}
		if (::IsWindow(view_->m_progressBar.m_hWnd))
		{
			view_->m_progressBar.SetPos(now);
			if ((now == 100) && info) //最后一次,info不能为空
			{
				//此处重新计算
				//model.set_saved_string();
				
				set_ui_state(3); //完成，调整界面
				
				model_->UpdateSavedInfo(installed_info_);
				view_->SetDlgItemTextW(IDC_STATIC_SAVED, installed_info_.c_str());
			
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
		if (::IsWindow(view_->m_progressBar.m_hWnd)) 
			view_->m_progressBar.SetPos(0);
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