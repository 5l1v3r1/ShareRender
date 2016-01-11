#include "TaskQueue.h"
#include "../LibCore/InfoRecorder.h"


namespace cg{
	namespace core{
#ifndef USE_CLASS_THREAD
DWORD TaskQueue::QueueProc(LPVOID param){
	TaskQueue * taskq = (TaskQueue *)param;

	IdentifierBase * t = NULL;
	taskq->init();

	infoRecorder->logError("[QueueProc]: wait the event.\n");
	WaitForSingleObject(taskq->getEvent(), INFINITE);
	taskq->lock();

	while(true){
		while(!taskq->isDone()){
#ifdef ENABLE_QUEUE_LOG
			infoRecorder->logTrace("[TaskQueue]: loop deal task, context:%p, task count:%d.\n", taskq->ctx, taskq->count);
#endif
			infoRecorder->logError("[TaskQueue]: loop deal task, context:%p, task count:%d.\n", taskq->ctx, taskq->count);
			t = taskq->getObj();
			if(!t){
				infoRecorder->logError("[TaskQueue]: get NULL front, task queue has:%d.\n", taskq->getCount());
				break;

			}
			if(t->sync == true){
				taskq->unlock();
				infoRecorder->logError("[TaskQueue]: the task is synchronized, %s.\n", typeid(*t).name());
			}
			else{
				// do the work here

				if(taskq->qStatus == QUEUE_CREATE){
#ifdef ENABLE_QUEUE_LOG
					infoRecorder->logTrace("[QueueProc]: create, check creation.\n");
					t->print();
#endif
					t->checkCreation(taskq->ctx);
					if(t->stable){
						t->checkUpdate(taskq->ctx);
					}
					//t->sendCreation();
				}
				else if(taskq->qStatus == QUEUE_UPDATE){
#ifdef ENABLE_QUEUE_LOG
					infoRecorder->logTrace("[QueueProc]: update, check update.\n"); 
					t->print();
#endif
					t->checkCreation(taskq->ctx);
					t->checkUpdate(taskq->ctx);
				}else{
					infoRecorder->logError("[QueueProc]: task status is INVALID.\n");
				}
				//t->sendUpdate();
			}

			taskq->popObj();
		}
		//infoRecorder->logError("[QueueProc]: wait the task event.\n");
		WaitForSingleObject(taskq->getEvent(), INFINITE);
		taskq->lock();
	}
}
#else  // USE_CLASS_THREAD

		inline IdentifierBase *TaskQueue::getObj(){
			return taskQueue.front();
		}
		inline void TaskQueue::popObj(){
			taskQueue.pop();
			EnterCriticalSection(&cs);
			count--;
			LeaveCriticalSection(&cs);
		}
		inline int TaskQueue::getCount(){
			int ret = 0;
			EnterCriticalSection(&cs);
			ret = count;
			LeaveCriticalSection(&cs);
			return ret;
		}
		inline void TaskQueue::setStatus(QueueStatus s){
			qStatus = s;
		}
		inline QueueStatus TaskQueue::getStatus(){
			return qStatus;
		}
		inline void TaskQueue::setContext(void *c){
			ctx = c;
		}
		bool TaskQueue::reset(){

		}

		void TaskQueue::add(IdentifierBase *obj){
			if(QUEUE_INVALID == qStatus){
				return;
			}else if(QUEUE_UPDATE == qStatus){

			}
			EnterCriticalSection(&cs);
			count++;
			taskQueue.push(obj);

			LeaveCriticalSection(&cs);
		}


		inline bool TaskQueue::isDone(){
			bool ret = false;
			EnterCriticalSection(&cs);
			ret = taskQueue.empty();
			LeaveCriticalSection(&cs);
			return ret;
		}
		inline void TaskQueue::lock(){

		}
		inline void TaskQueue::unlock(){

		}


		// on start, do the initialization
		BOOL TaskQueue::onThreadStart(){

		}
		// the function should be done in a thread slice
		BOOL TaskQueue::run(){
			IdentifierBase * t = NULL;

			while(!isDone()){
#ifdef ENABLE_QUEUE_LOG
				infoRecorder->logTrace("[TaskQueue]: loop deal task, count: %d.\n", count);
#endif // ENABLE_QUEUE_LOG
				t = getObj();
				if(!t){
					infoRecorder->logError("[TaskQueue]: get NULL front, task queue has: %d.\n", count);
					break;
				}
				if(true == t->sync){
					infoRecorder->logError("[TaskQueue]: the task is synchronized, %s.\n", typeid(*t).name());
				}
				else{
					// do the work
					if(QUEUE_CREATE == qStatus){
						// do the creation
#ifdef ENABLE_QUEUE_LOG
						infoRecorder->logTrace("[TaskQueue]: create, check creation.\n");
						t->print();
#endif  // ENABLE_QUEUE_LOG
						t->checkCreation(ctx);
						if(t->stable)
							t->checkUpdate(ctx);
					}else if(QUEUE_UPDATE == qStatus){
#ifdef ENABLE_QUEUE_LOG
						infoRecorder->logTrace("[TaskQueue]: update, check update.\n");
						t->print();
#endif
						t->checkCreation(ctx);
						t->checkUpdate(ctx);
					}
					else{
						infoRecorder->logError("[TaskQueue]: task status is invalid.\n");
					}
				}
			}
			popObj();
		}
#endif  // USE_CLASS_THREAD
	}
}