#pragma once
#include <Windows.h>
#include <string>

class Process
{
private:
	HANDLE event_handle;
	STARTUPINFOA startup_info;
	PROCESS_INFORMATION process_info;
	bool joined;
	bool obsolete;
public:
	Process(const char* filename, int number);
	Process(const Process& other) = delete;
	Process(Process&& other) noexcept;
	Process& operator = (const Process& other) = delete;
	Process& operator = (Process&& other) = delete;
	void join();
	void notify();
	~Process();
private:
	struct str
	{
		char* string;
		str(int number);
		str(const std::string& st);
		~str();
	};

};