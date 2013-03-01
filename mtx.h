#ifndef MTX_H
#define MTX_H
#include <pthread.h>

class Mtx{
public:
	Mtx() {
		pthread_mutex_init( &m_mutex, NULL );
	}
	void lock() {
		pthread_mutex_lock( &m_mutex );
	}
	void unlock() {
		pthread_mutex_unlock( &m_mutex );
	}
	
	class ScopedLock
	{
		Mtx& _mutex;
	public:
		ScopedLock(Mtx& mutex)
		: _mutex(mutex)
		{
			_mutex.lock();
		}
		~ScopedLock()
		{
			_mutex.unlock();
		}
	};
private:
	pthread_mutex_t m_mutex;
};
#endif