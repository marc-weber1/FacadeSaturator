/*
    WDL - ringbuf.h
    Copyright (C) 2005 Cockos Incorporated
    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software. (THIS VERSION HAS BEEN MODIFIED FROM CIRCBUF.H)
    3. This notice may not be removed or altered from any source distribution.
    
*/

/*
  This file provides a simple class for a circular FIFO queue of bytes. 
*/

#pragma once

#include "heapbuf.h"

class WDL_RingBuf
{
public:
  WDL_RingBuf() : m_hb(4096 WDL_HEAPBUF_TRACEPARM("WDL_RingBuf"))
  {
    m_inbuf = m_wrptr = 0;
  }
  ~WDL_RingBuf()
  {
  }
  void SetSize(int size)
  {
    m_inbuf = m_wrptr = 0;
    m_hb.Resize(size,true);
  }
  int GetSize(){
	  return m_hb.GetSize();
  }
  void Reset() { m_inbuf = m_wrptr = 0; }
  int Add(const void *buf, int l)
  {
    const int bf = m_hb.GetSize();
    if (l > bf) l = bf;
    if (l > 0)
    {
      const int wr1 = m_hb.GetSize()-m_wrptr;
      if (wr1 < l)
      {
        memmove((char*)m_hb.Get() + m_wrptr, buf, wr1);
        memmove(m_hb.Get(), (char*)buf + wr1, l-wr1);
        m_wrptr = l-wr1;
      }
      else 
      {
        memmove((char*)m_hb.Get() + m_wrptr, buf, l);
        m_wrptr = wr1 == l ? 0 : m_wrptr+l;
      }
      if(m_inbuf<bf) m_inbuf += l;
    }
    return l;
  }
  int Peek(void *buf, int offs, int len) const 
  {
    if (offs<0) return 0;
    const int ibo = m_inbuf-offs;
	const int bf = m_hb.GetSize();
	if(len > bf) len = bf;
    if (len > 0)
    {
      int rp = m_wrptr - ibo;
      if (rp < 0) rp += m_hb.GetSize();
      const int wr1 = m_hb.GetSize() - rp;
      if (wr1 < len)
      {
        memmove(buf,(char*)m_hb.Get()+rp,wr1);
        memmove((char*)buf+wr1,m_hb.Get(),len-wr1);
      }
      else
      {
        memmove(buf,(char*)m_hb.Get()+rp,len);
      }
    }
    return len;
  }
  int NbFree() const { return m_hb.GetSize() - m_inbuf; } // formerly Available()
  int NbInBuf() const { return m_inbuf; }

private:
  WDL_HeapBuf m_hb;
  int m_inbuf, m_wrptr;
} WDL_FIXALIGN;


template <class T>
class WDL_TypedRingBuf
{
public:

    WDL_TypedRingBuf() {}
    ~WDL_TypedRingBuf() {}

    void SetSize(int size)
    {
        mBuf.SetSize(size * sizeof(T));
    }
	
	void Fill(T value){ //VERY UNOPTIMIZED; MEMSET DIRECTLY?
		const int size = mBuf.GetSize();
		T* zerobuf = new float[size];
		for(int i=0; i<size; i++){
			zerobuf[i] = value;
		}
		mBuf.Add(zerobuf, size * sizeof(T));
		free(zerobuf);
	}

    void Reset()
    {
        mBuf.Reset();
    }

    int Add(const T* buf, int l)
    {
        return mBuf.Add(buf, l * sizeof(T)) / sizeof(T);
    }

    int Peek(T* buf, int offs, int len) const{
        return mBuf.Peek((void*) buf, offs*sizeof(T), len*sizeof(T)) / sizeof(T);
    }

private:
    WDL_RingBuf mBuf;
} WDL_FIXALIGN;