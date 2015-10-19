#pragma once
#include "Task.h"
#include <thread>
#include <condition_variable>
#include <mutex>

namespace cliqCity
{
	namespace multicore
	{
		typedef std::condition_variable			Notifier;
		typedef std::thread						Thread;
		typedef std::mutex						Mutex;
		typedef std::unique_lock<Mutex>			Lock;

		template<class Allocator>
		class TaskDispatcher
		{
		public:
			Notifier	mTaskNotifier;
			Mutex		mQueueLock;
			Allocator*	mAllocator;
			Thread*		mThreads;
			uint8_t		mThreadCount;
			uint32_t	mTaskCount;
			bool		mTasksAvailable;

			TaskDispatcher(Allocator* allocator, Thread* threads, uint8_t threadCount) : mAllocator(allocator), mThreads(threads), mThreadCount(threadCount)
			{
				
			}

			TaskDispatcher() : TaskDispatcher(nullptr, nullptr, 0)
			{

			}

			~TaskDispatcher()
			{

			}

			void Start()
			{
				for (int i = 0; i < mThreadCount; i++)
				{
					mThreads[i] = Thread::thread(WaitForAvailableTasks);
				}
			}

			void SetAllocator(Allocator* allocator)
			{
				mAllocator = allocator;
			}

			inline TaskID GetTaskID(Task* task) const
			{
				return task - reinterpret_cast<Task*>(mAllocator.Begin());
			}

			inline Task* GetTask(const TaskID& taskID) const
			{
				return reinterpret_cast<Task*>(mAllocator.Begin()) + taskID;
			}

			TaskID AddTask(const TaskData& data, TaskKernel kernel)
			{
				Task* task = AllocateTask();
				task->mData		= data;
				task->mKernel	= kernel;

				QueueTask(task);

				return GetTaskID(task);
			}

		private:
			inline Task* AllocateTask()
			{
				Task* task = mAllocator.Allocate(sizeof(Task), alignof(Task), 0);
				return task;
			}

			inline void QueueTask(Task* task)
			{
				mTasksAvailable = true;
				mTaskNotifier.notify_all();
			}

			inline void ExecuteTask(Task* task)
			{
				(task->mKernel)(task->mData);
			}

			inline Task* WaitForAvailableTasks()
			{
				Lock lock(mQueueLock);
				while (!mTasksAvailable) 
				{
					mTaskNotifier.wait(lock);
				}


			}
		};
	}
}