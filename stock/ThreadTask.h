#ifndef thread_task_h
#define thread_task_h
#pragma once
#include <thread>
#include <map>

class ThreadTask
{
public:
	ThreadTask();
	~ThreadTask();

	static std::map<std::thread::id, std::wstring> thread_map;
	static void Add(std::thread::id, std::wstring);
	static void Remove(std::thread::id);
	static bool is_empty();
     
};

#endif