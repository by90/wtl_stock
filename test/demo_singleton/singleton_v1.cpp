#include "singleton_v1.h"


singleton_v1::singleton_v1(int _id) :id(_id)
{
}
singleton_v1 * singleton_v1::GetInstance(int _id)
{
	if (pInstance_ == nullptr)  //判断是否第一次调用  
		pInstance_ = new singleton_v1(_id);
	return pInstance_;
}

singleton_v1 * singleton_v1::pInstance_ = nullptr;