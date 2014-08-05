#include "stdafx.h"
#include "ThreadTask.h"


ThreadTask::ThreadTask()
{
}


ThreadTask::~ThreadTask()
{
}

std::map<std::thread::id, std::wstring> ThreadTask::thread_map;

void ThreadTask::Add(std::thread::id _id, std::wstring _title)
{
	thread_map[_id]= _title;
}
void ThreadTask::Remove(std::thread::id _id)
{
	auto current = thread_map.find(_id);
	if (current != thread_map.end())
		thread_map.erase(current);
}

bool ThreadTask::is_empty()
{
	if (thread_map.size()==0)
		return true;
	return false;
}