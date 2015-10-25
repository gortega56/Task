#include "TaskDispatcher.h"
#include "PoolAllocator.h"
#include <stdio.h>
#include <sstream>

#define TASK_COUNT				50
#define THREAD_COUNT			5
#define TASK_POOL_SIZE			1024
#define SYNCHRONIZATION_RATE	10
#define SERIAL_LOOP_COUNT		50

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
	srand(static_cast<int>(time(&now)));
	
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

	printf("Concurrency Test: %i Tasks %i Threads\n", TASK_COUNT, THREAD_COUNT);

	for (int i = 0; i < TASK_COUNT; i++)
	{
		TaskID taskID = dispatchQueue.AddTask(data[i], PrintTask);
	}

	dispatchQueue.Synchronize();

	printf("Synchronization Test: %i Tasks %i Threads Synchronization Rate %i\n", TASK_COUNT, THREAD_COUNT, SYNCHRONIZATION_RATE);

	for (int i = 0; i < TASK_COUNT; i++)
	{
		TaskID taskID = dispatchQueue.AddTask(data[i], PrintTask);
		if ((i + 1) % SYNCHRONIZATION_RATE == 0)
		{
			dispatchQueue.Synchronize();
		}
	}

	printf("Serial Wait Test: %i Tasks %i Threads Loops %i\n", TASK_COUNT, THREAD_COUNT, SERIAL_LOOP_COUNT);

	for (int j = 0; j < SERIAL_LOOP_COUNT; j++)
	{
		for (int i = 0; i < TASK_COUNT; i++)
		{
			TaskID taskID = dispatchQueue.AddTask(data[i], PrintTask);
			dispatchQueue.WaitForTask(taskID);
		}
	}

	printf("Start and Stop Test: %i Tasks %i Threads\n", TASK_COUNT, THREAD_COUNT);

	int i;
	for (i = 0; i < TASK_COUNT / 2; i++)
	{
		TaskID taskID = dispatchQueue.AddTask(data[i], PrintTask);
	}

	dispatchQueue.Pause();

	printf("Pausing Queue\n");

	for (i; i < TASK_COUNT; i++)
	{
		TaskID taskID = dispatchQueue.AddTask(data[i], PrintTask);
	}

	dispatchQueue.Start();

	getchar();
}