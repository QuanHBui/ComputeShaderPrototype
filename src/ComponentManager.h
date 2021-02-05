#pragma once

#ifndef COMPONENT_MANAGER
#define COMPONENT_MANAGER

#include <vector>

template<typename T>
class ComponentManager
{
public:
	int emplaceBack(T t)
	{
		mDataContainer.emplace_back(t);
		return mAvailableIdx++;
	}

private:
	int mAvailableIdx = 0;
	std::vector<T> mDataContainer;
};

#endif // COMPONENT_MANAGER