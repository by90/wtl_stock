#include <string>
#include <memory>
#include <thread>

#ifndef STOCK_STOCK_IMPORT_VIEW_MODEL_H
#define STOCK_STOCK_IMPORT_VIEW_MODEL_H
class ImportView;
class ImportModelBase;
class ImportViewModel
{
public:

	ImportViewModel(ImportView *_view=nullptr);
	ImportView *view_;
	std::shared_ptr<ImportModelBase> model_;

	std::wstring installed_info_=L"";
	std::wstring selected_info_=L"";
	std::wstring selected_file_ = L"";
	std::wstring progress_info_ = L"";
	unsigned long file_count_ = 0; //需要吗？

	int progress_total_=100, progress_now_=0;
	int import_catalog_= 0;
	int ui_state_ = 0;
	
	std::thread::id  import_id_;

	//初始化，由View的InitDialog调用
	void Init();

	//选择文件，并检查合法性
	void SelectFile();

	void CheckFile();
	//安装，并使用回调更新进度文本控件和进度条
	void ImportFile(std::function<void(const char *, int)> func);

	void RunImportFile();

	//改变导入文件的类型，将状态设为0
	void ChangeImportFileType(WORD wID);

	//改变View的状态，由此设定各控件的可用性和可见性
	void set_ui_state(int _ui_state);

	//删除全部日线和代码表
	void RemoveInstalledQuote();

	//获取已安装数据字符串
	void set_installed_info(unsigned long _begin, unsigned long _end);
};
#endif