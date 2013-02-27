//Copyright (c) 2006 Bruno van Dooren
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.


// QueueDemo.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "Queue.h"
/*
bool g_Alive = true;
using namespace std;

//this thread will continuously consume elements from the queue
unsigned int __stdcall ReaderThread(void* ctx)
{
	CQueue<double> *localQueue = (CQueue<double> *) ctx;

	double d;
	while(true == g_Alive)
	{
		if(localQueue->PopElement(d))
		{
			cout << "Read value: " << d << endl;
			Sleep(50);
		}
	}

	return 0;
}

//this thread will continuously insert elements from the queue.
//this will happen in bursts.
unsigned int __stdcall WriterThread(void* ctx)
{
	CQueue<double> *localQueue = (CQueue<double> *) ctx;

	double d=0;
	while(true == g_Alive)
	{
		while(localQueue->PushElement(d))
		{
			cout << "Write value: " << d << endl;
			d++;
		}
		Sleep(1000);
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
  //instantiate a new queue
	CQueue<double> localQueue;

	unsigned int readerThreadId;
	unsigned int writerThreadId;
  HANDLE readerThread = NULL;
  HANDLE writerThread = NULL;

  //start the producer and consumer
	readerThread = (HANDLE)_beginthreadex(
    NULL, 0, &ReaderThread, (void*)&localQueue, 0, &readerThreadId);
	writerThread = (HANDLE)_beginthreadex(
    NULL, 0, &WriterThread, (void*)&localQueue, 0, &writerThreadId);
	
  //finish if the threads have not been created succesully.
  if((NULL == readerThread) || (NULL == readerThread))
	  g_Alive = false;
  else
  {
    //wait until the user presses enter.
    char dummy;
    cin >> dummy;
    g_Alive = false;
  }

  //wait for the threads to end
  if(NULL != readerThread)
  {
    WaitForSingleObject(readerThread, INFINITE);
    CloseHandle(readerThread);
  }
  if(NULL != writerThread)
  {
    CloseHandle(writerThread);
    WaitForSingleObject(writerThread, INFINITE);
  }
	return 0;
}
*/
