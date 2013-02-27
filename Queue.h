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


//a thread-safe consumer producer queue that uses no synchronization.
//it is as simple as possible for demonstration purposes.
//#include <stdio.h>
template<class T> class CQueue
{
private:
	volatile int m_Read;
	volatile int m_Write;
	static const int Size = 1000000;
	volatile T m_Data[Size];

public:
	CQueue()
	{
		m_Read = 0;
		m_Write = 0;
	}

  //push a new element in the circular queue
	bool PushElement(T &Element)
	{
		int nextElement = (m_Write + 1) % Size;
		if(nextElement != m_Read) 
		{
			m_Data[m_Write] = Element;
			m_Write = nextElement;
			return true;
		}
		else
			return false;
	}

  //remove the next element from the circualr queue
	bool PopElement(T &Element)
	{
		if(m_Read == m_Write){
			//printf("\nNothing Returned NULL POPPED!!!\n");
			return false;
		}

		int nextElement = (m_Read + 1) % Size;

		Element = m_Data[m_Read];
		m_Read = nextElement;
		return true;
	}
	int size(){return m_Write-m_Read;};
};