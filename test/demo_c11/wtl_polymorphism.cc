#include "gtest/gtest.h"
using namespace std;


//wtl的多态机制：使用模板而非虚函数，省掉了vtable的开销

//定义基类
template <class T>
class base_class
{
public:
	//由RunWhoAmI和WhoAmI两个函数，实现WhoAmI的多态，子类覆盖后者，前者调用子类的WhoAmI
	std::string RunWhoAmI()
	{
		T *ptr = static_cast<T*>(this); //将基类强制转换成T类，也就是子类的类型
		//也可写成：T* ptr = (T *)this;
		return ptr->WhoAmI();//执行子类的WhoAmI函数
	}

	//直接返回string是可行的，实际上是拷贝一份给外部对象
	std::string WhoAmI()
	{
		return "base_class";
	}

};

//定义子类，覆盖WhoAmI函数
//继承自base_class，模板参数将自己的类名传递进去，所谓的递归模板
class child_class :public base_class<child_class>
{
public:
	//覆盖基类的WhoAmI函数
	std::string WhoAmI()
	{
		return "child_class";
	}
};

//定义子类，不覆盖WhoAmI函数
class child :public base_class<child>
{

};


//wtl的递归模板的多态机制
TEST(demo,wtl_polymorphism)
{
	child_class A;
	child B;

	//执行父类的RuncWhoAmI函数，事实上调用了子类的WhoAmI函数
	EXPECT_TRUE("child_class" == A.RunWhoAmI());

	//未覆盖父类的WhoAmI函数，因此，RunWhoAmI调用父类的WhoAmI函数
	EXPECT_TRUE("base_class" == B.RunWhoAmI());
	//这种机制仅实现了多态，避免虚拟表开销，但不易用统一的基类指针指向派生类实例
	//仅仅是提升性能的技巧
}