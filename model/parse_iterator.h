#include <iterator>
#include <functional>
#include <istream>

//头文件不能在宏之后定义？
#ifndef parse_iterator_h
#define parse_iterator_h
//基于迭代器的迭代器
//由于Source可指定默认值，我们需要放在后面
//T是要解析出来的类型
template <typename T, typename source_iterator_type>
class parse_iterator :std::iterator<std::input_iterator_tag, T>
{
public:
	//由于是模板类，self_type应如下定义
	using self_type = parse_iterator<T, source_iterator_type>;

	//正常的构造函数
	parse_iterator(source_iterator_type _begin, source_iterator_type _end, value_type &&_value):
		begin_(_begin), end_(_end), value_(_value), ptr_(&value_)
	{

	}
	//iterator
	parse_iterator() = default;//指示编译器生成默认的构造函数
	parse_iterator(const self_type&); //使用另一个迭代器构造
	~parse_iterator(){};
	self_type& operator=(const self_type&);


	self_type& operator++(); //prefix increment
	reference operator*() const;
	friend void swap(self_type& lhs, self_type& rhs); //C++11 I think

	//input_iterator
	self_type operator++(int); //postfix increment后置++
	//value_type operator*() const;
	pointer operator->() const;
	friend bool operator==(const self_type& _first, const self_type& _next)
	{
		//nullptr不能执行*操作，所以需要判断
		//如果比较的两个，至少有一个为nullptr
		if (!_first.ptr_ || !_next.ptr_) return (_first.ptr_ == _next.ptr_);

		//如果都不是
		return (*_first.ptr_ == *_next.ptr_);
		
	};
	friend bool operator!=(const self_type&, const self_type&);
protected:
	pointer ptr_ = nullptr;//初始化为nullptr，表示end
	value_type value_;//仅在构造函数中使用
	source_iterator_type begin_; //传入迭代器的开始值
	source_iterator_type end_;//传入迭代器的结束值
	std::function<void(reference)> advance_; //前置++操作
	std::function<bool(value_type, value_type)> equal_to_; //相等操作
};
#endif