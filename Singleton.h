/* Start Header -------------------------------------------------------
Copyright (C) 2019
DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute
of Technology is prohibited.
Language:<C++>
Platform:<windows 10>
Project:<GAM 541, Void Engine>
Author:<Wang Zhongqiu>
-End Header --------------------------------------------------------*/
#pragma once
template <class T>
class Singleton
{
private:
	static T* mInstatce;
	Singleton(const Singleton<T>& c) {}
	Singleton &operator=(const Singleton<T>& c) {}
	template <class T>
	class DeleteSingleton
	{
	public:
		DeleteSingleton() {
			int a = 0;
		}
		~DeleteSingleton()
		{
			if (Singleton::mInstatce)
			{
				delete Singleton::mInstatce;
				Singleton::mInstatce = nullptr;
			}
		}
	};
protected:
	Singleton() {
		static DeleteSingleton<T> deleteSingleton;
	}
	virtual ~Singleton() {}
public:
	
	static T* GetInstance()
	{
		if (mInstatce == nullptr) 
			mInstatce = new T();
		return mInstatce;
	};

	
};
template <class T>
T* Singleton<T>::mInstatce = nullptr; 