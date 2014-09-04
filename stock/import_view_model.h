#include <string>
#include "QuoteView.h"

#ifndef STOCK_STOCK_IMPORT_VIEW_MODEL_H
#define STOCK_STOCK_IMPORT_VIEW_MODEL_H
class QuoteViewModel
{
public:
	CQuoteView *view_;
	//BaseModel *model;

	std::wstring install_data_=L"";
	std::wstring selected_data_=L"";
	std::wstring selected_file_ = L"";
	std::wstring progress_info_ = L"";

	bool is_file_validate = false;
	int progress_total=100, progress_now=0;
	int import_catalog = 0;
	int ui_state = 0;

	//初始化，由View的InitDialog调用
	void Init()
	{

	}

	//选择文件，并检查合法性
	void SelectFile()
	{

	}

	//安装，并使用回调更新进度文本控件和进度条
	void ImportFile(std::function<void(const char *, int)> func)
	{

	}

	//改变导入文件的类型，将状态设为0
	void ChangeImportFileType()
	{

	}

	//改变View的状态，由此设定各控件的可用性和可见性
	void set_ui_state()
	{

	}

	//删除全部日线和代码表
	void RemoveInstalledQuote()
	{

	}
};
#endif