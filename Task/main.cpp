#include "TaskDispatcher.h"
#include "PoolAllocator.h"
#include <stdio.h>
#include <sstream>

#define TASK_COUNT		50
#define THREAD_COUNT	5
#define TASK_POOL_SIZE	1024

using namespace cliqCity::multicore;

void PrintTask(const TaskData& data)
{
	std::stringstream ss;
	std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 4));
	int s = *reinterpret_cast<int*>(data.mKernelData);

	ss << "Task " << s << " executed ";
	for (int i = 0; i < s; i++)
	{
		ss << "*";
	}
	ss << std::endl;

	printf("%s", ss.str().c_str());
}

int main(int argc, int* argv[])
{
	time_t now;
	srand(time(&now));

	char memory[TASK_POOL_SIZE];
	Thread threads[THREAD_COUNT];
	TaskData data[TASK_COUNT];
	int taskNumbers[TASK_COUNT];

	taskNumbers[0] = 0;
	data[0].mKernelData = &taskNumbers[0];

	for (int i = 1; i < TASK_COUNT; i++)
	{
		taskNumbers[i] = i;
		data[i].mKernelData = &taskNumbers[i];
	}

	TaskDispatcher dispatchQueue(threads, THREAD_COUNT, memory, TASK_POOL_SIZE);
	dispatchQueue.Start();


	while (true)
	{
		for (int i = 0; i < TASK_COUNT; i++)
		{
			TaskID taskID = dispatchQueue.AddTask(data[i], PrintTask);
			dispatchQueue.WaitForTask(taskID);
		}
	}
	

	getchar();
}