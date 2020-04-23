#include "Process.h"
#include <vector>
#include <conio.h>
#include <Windows.h>
#include <string>
#include <iostream>

int main()
{
	std::vector<Process> processes;
	char value = '0';
	while (value != 'q')
	{
		value = _getch();
		if (value == '+')
		{
		
			try
			{
				processes.emplace_back("slave.exe", processes.size());
			}
			catch (DWORD a)
			{
				std::cout << a << std::endl;
			}
		}
		else if (value == '-')
		{
			
			if (!processes.size())
			{
				std::cout << "no processes" << std::endl;
				continue;
			}
			processes[processes.size() - 1].notify();
			processes[processes.size() - 1].join();
			processes.pop_back();
		}
	}
	
	for (auto& i : processes)
	{
		i.notify();
		i.join();
	}
	return 0;
}