// Resources: http://blog.molecular-matters.com/2012/04/12/building-a-load-balanced-task-scheduler-part-2-task-model-relationships/

#pragma once
#include <stdint.h>

namespace cliqCity
{
	namespace multicore
	{
		struct TaskID
		{
			uint32_t mOffset;
			
			TaskID(uint32_t offset) : mOffset(offset) {};
		};

		struct TaskData
		{
			void* mKernelData;
		};
		
		typedef void(*TaskKernel)(const TaskData&);
		
		class Task
		{
		public:
			TaskID		mID;
			TaskData	mData;
			TaskKernel	mKernel;

			Task();
			~Task();
		};
	}
}



