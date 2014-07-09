#include <string>
#include <cerrno>
#include <fstream>
#include "base_parse.h"

#ifndef file_parse_h
#define file_parse_h


using namespace std;

template <typename T, typename source_iterator_type = char *, typename Iter = parse_iterator<T, source_iterator_type>>
class file_parse :public base_parse<T,source_iterator_type,Iter>
{
public:
	file_parse() = default;
	file_parse(T beginValue) :
		base_parse({}, {}, beginValue)
	{}

	/*//使用c风格的文件读写，是最快的方案
	size_t open(wstring filename)
	{
		FILE* file;
		errno_t err = _wfopen_s(&file, filename.c_str(), L"rb");
		if (err != 0)
			return 0;
		fseek(file, 0, SEEK_END);
		size_t len = ftell(file);
		rewind(file);

		if (this->block_) free(block_);
		//这里，若block_为其他类型迭代器，是否会出错???
		block_ = (source_iterator_type)malloc(this->size_);
		//如果是智能指针
		// if (block_) block_reset()//如果已经分配空间，将删除。若已经复制给另外的变量，则只是减少引用计数
		//分配空间，使用free作为delete函数
		//block_ = shared_ptr<char>((char *)malloc(len), free);
		fread_s((void *)block_.get(), len, len, 1, file);
		fclose(file);//关闭


		//设置股票数量和记录数量
		//TagNumber = *(unsigned long *)(all.get() + 8);
		//RecordNumber = (unsigned long)(len - 16 - 32 - TagNumber * 32) / 32;

		return len;

		//所谓弱引用，只引用，不计数。换句话说，如果是shared_ptr，则下面的两个reset了才删除空间。
		//weak_ptr<char> head(block_);
		//char *current = block_.get();
	}
	*/

	//打开时才读取，因为判断格式合法性无需载入全部文件
	bool open(const char *filename)
	{
	    std::ifstream in(filename, std::ios::in | std::ios::binary);
		if (in)
		{			
			in.seekg(0, std::ios::end);
			this->size_=in.tellg();
			if (this->block_) free(block_);
			//这里，若block_为其他类型迭代器，是否会出错???
			block_ =(source_iterator_type) malloc(this->size_);
			in.seekg(0, std::ios::beg);
			in.read(&block_[0],size_);//按char类型读取
			in.close();
			return true;
		}
		else
			return false;

		//不要抛出异常
		//throw(errno);
	}
protected:
	string path_ = {};
};
#endif