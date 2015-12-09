#ifndef __QUEUE_H__
#define __QUEUE_H__
// queue

namespace cg{
	namespace core{

		template<typename T> struct QueueItem{
			//friend class Queue<T>;
			// needs to access to item and next

			// private class: no public section
			QueueItem(const T &t): item(t), next(0){}
			T item;
			QueueItem * next;
		};

		template <typename T> class Queue{
			QueueItem<T> * head;
			QueueItem<T> * tail;

			CRITICAL_SECTION section;
			// destroy the whole queue
			void destroy(){
				while(!empty())
					pop();
			}

			void copy_elems(const Queue &org){
				for(QueueItem<T> * pt = org.head; pt; pt = pt->next)
					push(pt->item);
			}
			template<typename Iter> void copy_elems(Iter, Iter){

			}

		public:
			Queue():head(0), tail(0){
				InitializeCriticalSection(&section);
			}
			template<class It> Queue(It beg, It end): head(0), tail(0){
				copy_elems(beg, end);
			}
			~Queue(){
				destroy();
				DeleteCriticalSection(&section);
			}

			// copy
			Queue(const Queue & q): head(0), tail(0){
				InitializeCriticalSection(&section);
				copy_elems(q);
			}

			// =
			Queue & operator=(const Queue &q){
				head = 0;
				tail = 0;
				copy_elems(q);
				return *this;
			}
			template<class Iter> void assign(Iter begin, Iter end){
				while(begin != end){
					push(begin);
					begin++;
				}
			}
			bool empty()const{
				bool ret = false;
				EnterCriticalSection((LPCRITICAL_SECTION)&section);
				ret = head == 0 ? true: false;
				LeaveCriticalSection((LPCRITICAL_SECTION)&section);
				return ret;
			}
			// get the head item, not remove
			T & front(){
				EnterCriticalSection((LPCRITICAL_SECTION)&section);
				T &tm = head->item;
				LeaveCriticalSection((LPCRITICAL_SECTION)&section);
				return tm;
			}

			const T & front() const{
				EnterCriticalSection((LPCRITICAL_SECTION)&section);
				const T & tm = head->item;
				LeaveCriticalSection((LPCRITICAL_SECTION)&section);
				return tm;
			}
			// just remove the head
			void pop(){
				QueueItem<T> * p = head;
				head = head->next;
				delete p;
			}

			// push to the tail
			void push(const T &val){
				// allocate new QueueItem object
				QueueItem<T> * pt = new QueueItem<T>(val);
				// put item onto existing queue
				if(empty())
					head = tail = pt; // only on elem
				else
				{
					EnterCriticalSection((LPCRITICAL_SECTION)&section);
					tail->next = pt;
					tail = pt;
					LeaveCriticalSection((LPCRITICAL_SECTION)&section);

				}
			}

		};
	}
}

#endif