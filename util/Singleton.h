#ifndef _SINGLETON_H
#define _SINGLETON_H
#include <iostream>
 
//通用：模板单例
template<typename T>
class Singleton
{
public:
	static T& getInstance() {
		static T t;
		return t;
	}

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
protected:
	Singleton() = default;
	~Singleton() = default;
};

#endif
