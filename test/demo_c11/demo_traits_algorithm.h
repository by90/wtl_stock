#ifndef demo_stl_my_advance_h
#define demo_stl_my_advance_h
#include <iterator>
using namespace std;

//演示算法如何在编译期，使用traits得到迭代器类型，从而避免运行时判断的开销

//用来获取迭代器的类型，编译期根据迭代器类型选择不同的函数
// TEMPLATE FUNCTION my_Iter_cat
template<class _Iter> inline
typename iterator_traits<_Iter>::iterator_category my_Iter_cat(const _Iter&)
{	// return category from iterator argument

	//编译期执行，定义类型
	typename iterator_traits<_Iter>::iterator_category _Cat;
	
	//因为inline，相当于直接返回，没有函数运行开销
	return (_Cat);
};

// TEMPLATE FUNCTION advance
//output迭代器之外，四种迭代器均支持
//输入迭代器，多次++
template<class _InIt,
class _Diff> inline
	void my_Advance(_InIt& _Where, _Diff _Off, input_iterator_tag)
{	// increment iterator by offset, input iterators
		for (; 0 < _Off; --_Off)
			++_Where;
}

//forward迭代器，和输入迭代器相同
template<class _FwdIt,
class _Diff> inline
	void my_Advance(_FwdIt& _Where, _Diff _Off, forward_iterator_tag)
{	// increment iterator by offset, forward iterators
		for (; 0 < _Off; --_Off)
			++_Where;
}

//双向迭代器...
template<class _BidIt,
class _Diff> inline
	void my_Advance(_BidIt& _Where, _Diff _Off, bidirectional_iterator_tag)
{	// increment iterator by offset, bidirectional iterators
		for (; 0 < _Off; --_Off)
			++_Where;
		for (; _Off < 0; ++_Off)
			--_Where;
}

//随机迭代器
template<class _RanIt,
class _Diff> inline
	void my_Advance(_RanIt& _Where, _Diff _Off, random_access_iterator_tag)
{	// increment iterator by offset, random-access iterators
		_Where += _Off;
}

//编译期间，判断迭代器类型，调用前面不同的函数
//这里，函数的参数忽略掉了与traits相关的东西，换句话说，最终的使用要自然简单。
template<class _InIt,
class _Diff> inline
	void my_advance(_InIt& _Where, _Diff _Off)
{	// increment iterator by offset, arbitrary iterators
		my_Advance(_Where, _Off, my_Iter_cat(_Where));
}
#endif //