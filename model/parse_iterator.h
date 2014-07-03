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
		begin_(_begin), end_(_end), value_(_value)
	{}

	//默认构造函数
	parse_iterator() = default;//指示编译器生成默认的构造函数
	
	//拷贝构造函数
	//实现拷贝构造后，assign即可正常工作，无需重载=
	//self_type& operator=(const self_type&);
	parse_iterator(const self_type& _another):
		begin_(_another.begin_), end_(_another.end_), value_(_another.value_)
	{}

	//默认析构函数
	~parse_iterator() = default;

	//==操作符
	friend bool operator==(const self_type& _first, const self_type& _next)
	{
		return (_first.value_ == _next.value_);
	};

	//!=操作符
	friend bool operator!=(const self_type& _first, const self_type& _next)
	{
		return (_first.value_ != _next.value_);
	}


	//*操作符
	value_type  operator*() const
	{
		return value_;
	};

	self_type& operator++(); //prefix increment
	self_type operator++(int); //postfix increment后置++

	
	friend void swap(self_type& lhs, self_type& rhs); //C++11 I think

	//value_type operator*() const;
	pointer operator->() const;

protected:
	value_type value_;//仅在构造函数中使用

	//这里使用默认的{}来初始化，意味着传入迭代器应具备构造函数
	source_iterator_type begin_ = {}; //传入迭代器的开始值
	source_iterator_type end_ = {};//传入迭代器的结束值
};
#endif