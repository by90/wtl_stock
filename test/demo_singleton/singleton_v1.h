#pragma once

//第一步，实现singleton的基本概念
//第一步的问题：动态分配的对象，没有析构，虽然new只执行一次，但即使系统退出，也没有delete
//因此，用"清洁工"模式，建立一个私有嵌入对象解决问题。这种方式显然麻烦

class singleton_v1
{
public:
	static singleton_v1 * GetInstance(int); //要点2：只公开一个获取实例指针的方式

	//为了演示，增加一个int类型的id，多线程测试有用
	int id;
private:
	singleton_v1(int); //要点1：私有构造函数，这样使用该类，只能通过GetInstance获取
	static singleton_v1 *pInstance_;	

	//以下的代码：在系统退出时析构pInstance
	class garbo   //清洁工类，在系统退出前，garbo_成员变量自动析构，由此执行下面的delete，该类不在任何其他地方使用，因此私有嵌入类 
	{
	public:
		~garbo()
		{
			if (singleton_v1::pInstance_)
				delete singleton_v1::pInstance_;
		}
	};
	static garbo garbo_;  //程序结束时，会自动的析构静态成员变量，从而执行delete方法  
};

