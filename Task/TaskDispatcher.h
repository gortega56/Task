#pragma once
#include "Task.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace cliqCity
{
	namespace multicore
	{

		typedef std::queue<Task*>				Queue;
		typedef std::condition_variable			Signal;
		typedef std::thread						Thread;
		typedef std::mutex						Mutex;
		typedef std::unique_lock<Mutex>			UniqueLock;
		typedef std::lock_guard<Mutex>			ScopedLock;

		template<class Allocator>
		class TaskDispatcher
		{
		public:
			Queue		mTaskQueue;
			Signal		mTaskSignal;
			Mutex		mQueueLock;
			Mutex		mMemoryLock;
			Allocator	mAllocator;
			Thread*		mThreads;
			uint8_t		mThreadCount;
			bool        mIsProcessingTasks;
			void*		mMemory;

			TaskDispatcher(Thread* threads, uint8_t threadCount, void* memory, size_t size) : 
				mAllocator(memory, reinterpret_cast<char*>(memory) + size, sizeof(Task)), 
				mThreads(threads), 
				mThreadCount(threadCount),
				mIsProcessingTasks(false),
				mMemory(memory)
			{
				
			}

			TaskDispatcher() : TaskDispatcher(nullptr, 0, nullptr, 0)
			{
				
			}

			~TaskDispatcher()
			{
				Sync();
				mThreads = nullptr;
			}

			void Start()
			{
				if (mIsProcessingTasks)
				{
					return;
				}

				mIsProcessingTasks = true;
				for (int i = 0; i < mThreadCount; i++)
				{
					mThreads[i] = Thread::thread(&TaskDispatcher<Allocator>::ProcessTasks, this);
				}
			}

			void Pause()
			{
				mIsProcessingTasks = false;
			}

			void Sync()
			{
				for (int i = 0; i < mThreadCount; i++)
				{
					if (mThreads[i].joinable())
					{
						mThreads[i].join();
					}
				}
			}

			inline TaskID GetTaskID(Task* task) const
			{
				return TaskID(task - reinterpret_cast<Task*>(mMemory));
			}

			inline Task* GetTask(const TaskID& taskID) const
			{
				return reinterpret_cast<Task*>(mMemory) + taskID.mOffset;
			}

			TaskID AddTask(const TaskData& data, TaskKernel kernel)
			{
				Task* task = AllocateTask();
				task->mData		= data;
				task->mKernel	= kernel;

				QueueTask(task);

				return GetTaskID(task);
			}

			inline Task* AllocateTask()
			{
				Task* task = nullptr;
				{
					ScopedLock lock(mMemoryLock);
					task = reinterpret_cast<Task*>(mAllocator.Allocate());
				}

				return task;
			}

			inline void FreeTask(Task* task)
			{
				ScopedLock lock(mMemoryLock);
				mAllocator.Free(task);
			}

			inline void QueueTask(Task* task)
			{
				{
					UniqueLock lock(mQueueLock);
					mTaskQueue.push(task);
				}

				if (mIsProcessingTasks)
				{
					mTaskSignal.notify_all();
				}
			}

			inline void ExecuteTask(Task* task)
			{
				(task->mKernel)(task->mData);
			}

			inline void ProcessTasks()
			{
				while (mIsProcessingTasks)
				{
					Task* task = WaitForAvailableTasks();
					ExecuteTask(task);
					FreeTask(task);
				}
			}

			inline Task* WaitForAvailableTasks()
			{
				UniqueLock lock(mQueueLock);
				while (mTaskQueue.empty()) 
				{
					mTaskSignal.wait(lock);
				}

				Task* task = mTaskQueue.front();
				mTaskQueue.pop();
				
				return task;
			}
		};
	}
}