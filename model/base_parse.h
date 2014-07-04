#include "parse_iterator.h"
#ifndef base_parse_h
#define base_parse_h
template <typename T,typename source_iterator_type = char *>
class base_parse
{
public:
	using iterator = parse_iterator<T, source_iterator_type>;
	base_parse(source_iterator_type data, size_t size, T beginValue) :
		block_(data), size_(size), begin_(beginValue)
	{}
	base_parse() = default;


	iterator begin() const
	{
		return iterator(block_, block_ + size_, begin_);
	}

	iterator end() const
	{
		return iterator();
	}
protected:
	source_iterator_type block_ = {};   //内部维护一个char *对象，我们从该对象获取ValueType对象。
	size_t size_=0;
	T begin_ = {};
};

#endif