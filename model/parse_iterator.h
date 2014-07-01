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
	using self_type = parse_iterator;

	//定义advance的函数类型
	using advance_function = std::function<void(reference)>;
	using equal_to_function = std::function<bool(value_type)>;

	parse_iterator(source_iterator_type _being, source_iterator_type _end, value_type _value)
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
	friend bool operator==(const self_type&, const self_type&);
	friend bool operator!=(const self_type&, const self_type&);
private:
	pointer ptr_ = nullptr;//初始化为nullptr，表示end
	value_type value_;//仅在构造函数中使用
	source_iterator_type begin_; //传入迭代器的开始值
	source_iterator_type end_;//传入迭代器的结束值
	std::function<void(reference)> advance_; //前置++操作
	std::function<bool(value_type, value_type)> equal_to_; //相等操作
};
#endif