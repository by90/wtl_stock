
#include <vector>
#include <list>
//#include <algorithm> //stl算法
#include "gtest/gtest.h"
#include <iterator>
#include <istream>
using namespace std;

//以下是stdmymy_Advance的源码，拷贝到这里，将advance改为my_advance

//用来获取迭代器的类型，编译期根据迭代器类型选择不同的函数
// TEMPLATE FUNCTION my_Iter_cat
template<class _Iter> inline
typename iterator_traits<_Iter>::iterator_category
my_Iter_cat(const _Iter&)
{	// return category from iterator argument
	typename iterator_traits<_Iter>::iterator_category _Cat;
	return (_Cat);
}


// TEMPLATE FUNCTION advance
//output迭代器之外，四种迭代器均支持
//输入迭代器，多次++
template<class _InIt,
class _Diff> inline
	void my_Advance(_InIt& _Where, _Diff _Off, input_iterator_tag)
{	// increment iterator by offset, input iterators
#if _ITERATOR_DEBUG_LEVEL == 2
		if (_Off < 0)
			_DEBUG_ERROR("negative offset in advance");
#endif /* _ITERATOR_DEBUG_LEVEL == 2 */

		for (; 0 < _Off; --_Off)
			++_Where;
}

//forward迭代器，和输入迭代器相同
template<class _FwdIt,
class _Diff> inline
	void my_Advance(_FwdIt& _Where, _Diff _Off, forward_iterator_tag)
{	// increment iterator by offset, forward iterators
#if _ITERATOR_DEBUG_LEVEL == 2
		if (_Off < 0)
			_DEBUG_ERROR("negative offset in advance");
#endif /* _ITERATOR_DEBUG_LEVEL == 2 */

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
template<class _InIt,
class _Diff> inline
	void my_advance(_InIt& _Where, _Diff _Off)
{	// increment iterator by offset, arbitrary iterators
		my_Advance(_Where, _Off, my_Iter_cat(_Where));
}


//通过my_Advance理解算法和迭代器
TEST(StlDemo, lgorithm_and_iterator)
{
	//1.使用指针：两个int指针间的距离
	int intPtr[]={ 1, 2, 3, 4, 5 };
	int *next_ptr = &intPtr[0];
	my_advance(next_ptr,1);
	EXPECT_EQ(2, *next_ptr);

	//2.随机迭代器
	vector<int> vec = { 1, 2, 3, 4, 5 };
	auto next_vec =  vec.begin();
	my_advance(next_vec, 1);
	EXPECT_EQ(2, *next_vec); //另一个迭代器，等于begin+3，即第0个向后移动3次，则应该为第3个，值为4

	//3.list的双向迭代器
	list<int> list_int = { 1, 2, 3, 4, 5 };
	auto next_list = list_int.begin();
	next_list++;
	next_list++; //该迭代器前移两次，注意，由于不是随机迭代器，不能一次移动两次。
	EXPECT_EQ(3,*next_list);
	my_advance(next_list, 1);
	EXPECT_EQ(4,*next_list);

	//4.

	
}
