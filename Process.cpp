#include "Process.h"
#include <exception>
#include <string>


Process::Process(const char* filename, int number)
	:joined { false }, obsolete { false }
{	
	ZeroMemory(&startup_info, sizeof(startup_info));
	ZeroMemory(&process_info, sizeof(process_info));
	startup_info.cb = sizeof(startup_info);
	if (!(event_handle = CreateEventA(NULL, TRUE, FALSE, (std::string("lab2 ") + std::to_string(number)).c_str())))
	{
		throw GetLastError();
	}
	if (!CreateProcessA(nullptr, str(std::string{ filename } + ' ' + std::to_string(number)).string, NULL, NULL, false, 0, NULL, NULL, &startup_info, &process_info))
	{
		CloseHandle(event_handle);
		throw GetLastError();
	}
}

Process::Process(Process&& other) noexcept
	:event_handle{ other.event_handle }, startup_info{ other.startup_info }, process_info{ other.process_info }, joined{ other.joined }, obsolete { false }
{
	other.obsolete = true;
}

void Process::join()
{
	WaitForSingleObject(process_info.hProcess, INFINITE);
	joined = true;
}

void Process::notify()
{
	SetEvent(event_handle);
}

Process::~Process()
{
	if (obsolete)
	{
		return;
	}
	if (!joined)
	{
		throw std::exception{};
	}
	CloseHandle(event_handle);
	CloseHandle(process_info.hProcess);
	CloseHandle(process_info.hThread);
}

Process::str::str(int number)
{
	std::string st = std::to_string(number);
	string = new char[st.size() + 1];
	for (int i = 0; i < st.size(); ++i)
	{
		string[i] = st[i];
	}
	string[st.size()] = 0;
}

Process::str::str(const std::string& st)
{
	string = new char[st.size() + 1];
	for (int i = 0; i < st.size(); ++i)
	{
		string[i] = st[i];
	}
	string[st.size()] = 0;
}

Process::str::~str()
{
	delete[] string;
}
