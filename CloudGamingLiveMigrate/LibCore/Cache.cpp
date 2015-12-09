#include "Cache.hpp"
#include "CThread.h"
#include "InfoRecorder.h"

namespace cg{
	namespace core{

		list<void *> Cache::gcMem;
		CRITICAL_SECTION Cache::section;
		LPCRITICAL_SECTION Cache::pSection = NULL;
		HANDLE Cache::helperHandle = NULL;
		DWORD Cache::helperId = 0;
		int Cache::freeCount = 0;

		// free the give pointer using free();
		void WINAPI Cache::FreeMem(void *p){
			EnterCriticalSection(pSection);
			infoRecorder->logError("[Cache]:push back to list:%p.\n", p);
			gcMem.push_back(p);
			freeCount++;
			LeaveCriticalSection(pSection);
		}

		void WINAPI Cache::startHelper(){
			helperHandle = chBEGINTHREADEX(NULL, 0, CacheHelper, NULL, FALSE, &helperId);
		}

		DWORD WINAPI Cache::CacheHelper(LPVOID param){
			//Cache * cache = (Cache *)param;
			list<void *>::iterator it;
			bool empty = false;
			while(true){
				if(freeCount > 10000){

					while(!empty){
						infoRecorder->logTrace("[CacheHelper]: to free:%d.\n", freeCount);
						EnterCriticalSection(pSection);
						empty = gcMem.empty();
						free(gcMem.front());
						gcMem.pop_front();
						freeCount--;
						LeaveCriticalSection(pSection);
					}
#if 0
					for(it = gcMem.begin(); it != gcMem.end(); it++){
						infoRecorder->logError("[CacheHelper]: it:%p, to release %p\n", it, *it);
						free(*it);
					}
#endif
					//freeCount = 0;

				}
				else{
					Sleep(1);
				}
			}
			return 0;
		}

	}
}