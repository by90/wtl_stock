#include "stdafx.h"
#include "import_view.h"
#include "import_view_model.h"


ImportViewModel::ImportViewModel(ImportView *_view)
{
	view_ = _view;
}

//初始化，由View的InitDialog调用
void ImportViewModel::Init()
{

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
void ImportViewModel::set_ui_state()
{

}

//删除全部日线和代码表
void ImportViewModel::RemoveInstalledQuote()
{

}