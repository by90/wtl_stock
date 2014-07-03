#include "gtest/gtest.h"
#include <functional>
#include "parse_iterator.h" 

//如果要为一组测试(testSuit为blockTest)提供初始化和清理方法，则要定义该testSuit
class ParseIterator : public testing::Test {
protected:
	//testSuit的初始化函数
	static void SetUpTestCase() {};

	//testSuit的清理函数
	static void TearDownTestCase() {};

	//SetUpTestCase() 方法在第一个TestCase之前执行
	//TearDownTestCase() 方法在最后一个TestCase之后执行
	//SetUp()方法在每个TestCase之前执行
	//TearDown()方法在每个TestCase之后执行
	//在TestSuit的各测试案例中共享的一些资源，定义为static
};

int intArray[5]{1, 2, 3, 4, 5};
char *int_ptr = reinterpret_cast<char*>(intArray);

/*
stl规范中，input迭代器必须具备的能力：
1.Is copy-constructible, copy-assignable and destructible
x b(a);b=a
必须实现拷贝构造函数，需有析构函数
2.Can be compared for equivalence using the equality/inequality operators
(meaningful if both iterators are be in domain).
a == b;a != b
必须重载==和!=操作符
3.Can be dereferenced as an rvalue (if in a dereferenceable state).
*a;a->m
重载*和->操作符
4.Can be incremented (if in a dereferenceable state).
The result is either also dereferenceable or a past-the-end iterator.
The previous iterator value is not required to be dereferenceable after the increase.
++a
(void)a++
*a++
要重载前置和后置的++操作符
5.Its value type does not need to be assignable
t = u not required
它的value type不需要assignable，不需要不代表不许可。
6.c++ 11 only
Lvalues are swappable.
swap(a,b)
提供swap函数
*/

//默认构造函数，由编译器生成，不用实现代码
TEST_F(ParseIterator, construct_default)
{
	parse_iterator<int *, char *> iter;
}

//主要构造函数，传入两个迭代器和一个T类型的move
TEST_F(ParseIterator, construct_regular)
{
	//默认构造函数，由编译器生成，不用实现代码
	parse_iterator<int *, char *> iter(int_ptr,int_ptr+sizeof(int)*5,(int *)int_ptr );
}
//拷贝构造函数
TEST_F(ParseIterator, construct_copy)
{
	parse_iterator<int *, char *> iter(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	parse_iterator<int *, char *> iter_copy(iter);
	parse_iterator<int *, char *> iter_assign=iter;
	EXPECT_EQ(iter,iter_copy);
	EXPECT_EQ(iter, iter_assign);
}


//operator==
TEST_F(ParseIterator, operator_equal)
{
	//两个nullptr比较，应相等
	parse_iterator<int *, char *> iter;
	parse_iterator<int *, char *> empty_iter;
	EXPECT_EQ(iter, empty_iter);

	//两个相等的iterator
	parse_iterator<int *, char *> iter1(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	parse_iterator<int *, char *> iter2(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	EXPECT_EQ(true, iter1 == iter2);

	//不相等的两个迭代器
	parse_iterator<int *, char *> iter3(int_ptr, int_ptr + sizeof(int)* 5, (int *)(int_ptr + sizeof(int)));
	EXPECT_EQ(false, iter1 == iter3);

	//与默认构造的迭代器比较
	EXPECT_EQ(false, iter == iter3);
}

//operator!=
TEST_F(ParseIterator, operator_equal_not)
{
	//两个nullptr比较，应相等
	parse_iterator<int *, char *> iter;
	parse_iterator<int *, char *> empty_iter;
	EXPECT_EQ(iter, empty_iter);

	//两个相等的iterator
	parse_iterator<int *, char *> iter1(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	parse_iterator<int *, char *> iter2(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	EXPECT_EQ(false, iter1 != iter2);

	//不相等的两个迭代器
	parse_iterator<int *, char *> iter3(int_ptr, int_ptr + sizeof(int)* 5, (int *)(int_ptr + sizeof(int)));
	EXPECT_EQ(true, iter1 != iter3);

	//与默认构造的迭代器比较
	EXPECT_EQ(true, iter != iter3);
}

TEST_F(ParseIterator, operator_point)
{
	parse_iterator<int *, char *> iter(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	EXPECT_EQ(1, **iter);
	parse_iterator<int *, char *> iter1(int_ptr, int_ptr + sizeof(int)* 5, (int *)(int_ptr+sizeof(int)*2));
	EXPECT_EQ(3, **iter1);
	EXPECT_EQ((int *)(int_ptr),*iter);
}

TEST_F(ParseIterator, operator_increment_prefix)
{
	parse_iterator<int *, char *> iter(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	EXPECT_EQ(1, **iter);
	++iter;
	EXPECT_EQ(2, **iter);
	++iter;
	EXPECT_EQ(3, **iter);
	++iter;
	EXPECT_EQ(4, **iter);
	++iter;
	EXPECT_EQ(5, **iter);
}

TEST_F(ParseIterator, operator_increment_postfix)
{
	parse_iterator<int *, char *> iter(int_ptr, int_ptr + sizeof(int)* 5, (int *)int_ptr);
	EXPECT_EQ(1, **iter);

	iter++;
	EXPECT_EQ(2, **iter);

	auto iter1=(iter++); //iter1是新的独立对象
	EXPECT_EQ(3, **iter); //iter被加1
	EXPECT_EQ(3, **iter1);// iter1没有加..需要修改

	++iter1;
	EXPECT_EQ(4, **iter1);
	EXPECT_EQ(3, **iter);
}