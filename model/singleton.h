#ifndef singleton_h
#define singleton_h

template <class T>
class Singleton
{
public:
	template <typename... Args>
	static T& Instance(Args&&... args) {
		static T* instance = new T(std::forward<Args>(args)...);
		return *instance;
	}
};
#endif // singleton_h
