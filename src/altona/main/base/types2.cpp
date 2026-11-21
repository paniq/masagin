/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#include "base/types2.hpp"
#include "base/system.hpp"
#include "base/serialize.hpp"

/****************************************************************************/
/***                                                                      ***/
/***   Garbage Collection Memory Management                               ***/
/***                                                                      ***/
/****************************************************************************/

static sInt GCFlag=0;
static sInt GCCollecting=0;
static sArray<sObject *> *GCObjects;
static sArray<sObject *> *GCRoots;
static sThreadLock *GCLock=0;       // internal lock for safe mulithreaded access to gc data
static sThreadLock *GCUserLock=0;   // external lock for exclusive gc collection or application access

void sGCLock()
{
  GCUserLock->Lock();
}

void sGCUnlock()
{
  GCUserLock->Unlock();
}

void sAddRoot(sObject *o)
{
  if(o)
  {
    GCLock->Lock();
    GCRoots->AddTail(o);
    GCLock->Unlock();
  }
}

void sRemRoot(sObject *o)
{
  sObject *f;
  if(o)
  {
    GCLock->Lock();
    sFORALL(*GCRoots,f)        // be careful, sArray::Rem(object) will remove ALL occurences!
    {
      if(f==o)
      {
        GCRoots->RemAt(_i);
        break;
      }
    }
    GCLock->Unlock();
  }
}

void sCollect()
{
  GCFlag = 1;
}

void sCollector()
{
  if(!GCUserLock->TryLock())
    return; // locked by other thread, so skip collection
  GCLock->Lock();
  sObject *o;
  if(GCFlag && GCObjects && GCRoots)
  {
    GCCollecting = 1;
    GCFlag = 0;

    sFORALL(*GCObjects,o)
      o->NeedFlag = 0;
    sFORALL(*GCRoots,o)
      o->Need();
    sFORALL(*GCObjects,o)
      if(!o->NeedFlag)
        o->Finalize();
    for(sInt i=0;i<GCObjects->GetCount();)
    {
      o = (*GCObjects)[i];
      if(!o->NeedFlag)
      {
        GCObjects->RemAt(i);
        delete o;
      }
      else
      {
        i++;
      }
    }
    GCCollecting = 0;
  }
  GCLock->Unlock();
  sGCUnlock();
}

void sInitGC()
{
  GCLock = new sThreadLock;
  GCUserLock= new sThreadLock;
  GCObjects = new sArray<sObject *>;
  GCRoots = new sArray<sObject *>;
  GCRoots->HintSize(1024);
}

void sExitGC()
{
  sCollect();
  sCollector();
  sDelete(GCLock);
  sDelete(GCUserLock);
  sDelete(GCObjects);
  sDelete(GCRoots);
}

sADDSUBSYSTEM(GarbageCollector,0x10,sInitGC,sExitGC);

/****************************************************************************/

sObject::sObject()
{
  sObject *o = this;
  GCLock->Lock();
  GCObjects->AddTail(o);
  GCLock->Unlock();
}

sObject::~sObject()
{
  if(!GCCollecting)
  {
    GCLock->Lock();
    GCRoots->Rem(this);
    GCObjects->Rem(this);
    GCLock->Unlock();
  }
}

void sObject::Need() 
{ 
  if(this && !NeedFlag) 
  { 
    NeedFlag=1; 
    Tag(); 
  }
}

void sObject::Finalize()
{
}

/****************************************************************************/
/***                                                                      ***/
/***   A message system for sObject                                       ***/
/***                                                                      ***/
/****************************************************************************/

static sMessage sMessagePool[1024];
static sInt sMessageUsed=0;
static sThreadLock *sMessageLock;

void sInitMessage()
{
  sMessageLock = new sThreadLock;
}

void sExitMessage()
{
  delete sMessageLock;
}

sADDSUBSYSTEM(Message,0x11,sInitMessage,sExitMessage);

void sMessage::Send() const
{
  if(this)
  {
    switch(Type)
    {
    case 0x01: 
      if(Target)
        (Target->*Func1)(); 
      break;
    case 0x02: 
      if(Target) 
        (Target->*Func2)(Code); 
      break;
    case 0x03: 
      (*Func3)(Target);
      break;
    case 0x04: 
      (*Func4)(Target,Code); 
      break;
    case 0x11: 
    case 0x12:
      sFatal(L"can't send a drag-message (type 0x11 or 0x12)");
      break;
    }
  }
}

void sMessage::Post() const
{
  if(this && Target)
  {
    sVERIFY(sMessageUsed < sCOUNTOF(sMessagePool));
    sMessagePool[sMessageUsed++] = *this;
  }
}

void sMessage::PostASync() const
{
  sMessageLock->Lock();
  Post();
  sMessageLock->Unlock();
  sTriggerEvent();
}

void sMessage::Pump()
{
  sMessageLock->Lock();
  for(sInt i=0;i<sMessageUsed;i++)
    sMessagePool[i].Send();
  sMessageUsed = 0;
  sMessageLock->Unlock();
}

void sMessage::Drag(const struct sWindowDrag &dd) const
{
  switch(Type)
  {
  case 0x11:
    (Target->*Func11)(dd);
    break;
  case 0x12:
    (Target->*Func12)(dd,Code);
    break;
  default:
    sVERIFYFALSE;
  }
}

/****************************************************************************/
/***                                                                      ***/
/***   Text Buffer Class (dynamic string of some kind)                    ***/
/***                                                                      ***/
/****************************************************************************/

sTextBuffer::sTextBuffer()
{
  Init();
}

sTextBuffer::sTextBuffer(const sTextBuffer& tb)
{
  Init();
  Print(tb.Get());
}

sTextBuffer::sTextBuffer(const sChar* t)
{
  Init();
  Print(t);
}

sTextBuffer::~sTextBuffer()
{
  delete[] Buffer;
}

sTextBuffer& sTextBuffer::operator=(const sTextBuffer& tb)
{
  if(this != &tb)
  {
    Clear();
    Print(tb.Get());
  }
  return *this;
}

sTextBuffer& sTextBuffer::operator=(const sChar* t)
{
  if (t < Buffer || t >= Buffer+Alloc)
  {
    Clear();
    Print(t);
  }
  return *this;
}


template <class streamer> void sTextBuffer::Serialize_(streamer &s)
{
  sVERIFY(sizeof(sChar)==sizeof(sU16));
  sInt len = Used;
  s | len;
  if(s.IsReading())
  {
    SetSize(len);
    Used = len;
  }
  s.ArrayU16((sU16*)Buffer, Used);
  s.Align();
  Buffer[Used] = 0;
}

void sTextBuffer::Serialize(sReader &stream)
{
  Serialize_(stream);
}

void sTextBuffer::Serialize(sWriter &stream)
{
  Serialize_(stream);
}

void sTextBuffer::Init()
{
  Alloc = 1024;
  Used = 0;
  Buffer = new sChar[Alloc];
}

void sTextBuffer::Grow(sInt add)
{
  if(Used+add+1 >= Alloc)
  {
    SetSize(sMax(Alloc+add+1,Alloc*2));
  }
}

void sTextBuffer::Clear()
{
  Used = 0;
}

void sTextBuffer::SetSize(sInt count)
{
  if(Alloc<count)
  {
    sChar *n = new sChar[count];
    sCopyMem(n,Buffer,Used*sizeof(sChar));
    delete[] Buffer;

    Buffer = n;
    Alloc = count;
  }
}

const sChar *sTextBuffer::Get() const
{
  sVERIFY(Used<Alloc);
  const_cast<sChar*>(Buffer)[Used] = 0;
  return Buffer;
}

sChar *sTextBuffer::Get()
{
  sVERIFY(Used<Alloc);
  const_cast<sChar*>(Buffer)[Used] = 0;
  return Buffer;
}

void sTextBuffer::Insert(sInt pos,sChar c)
{
  sVERIFY(pos>=0 && pos<=Used);
  Grow(1);

  Used++;
  for(sInt i=Used;i>pos;i--)
    Buffer[i] = Buffer[i-1];
  Buffer[pos] = c;
}

void sTextBuffer::Insert(sInt pos,const sChar *c,sInt len)
{
  sVERIFY(pos>=0 && pos<=Used);
  if(len==-1) len = sGetStringLen(c);

  Grow(len);

  Used+=len;
  for(sInt i=Used;i>pos;i--)
    Buffer[i] = Buffer[i-len];
  for(sInt i=0;i<len;i++)
    Buffer[pos+i] = c[i];
}


void sTextBuffer::Delete(sInt pos)
{
  sVERIFY(pos>=0 && pos<Used);
  Used--;
  for(sInt i=pos;i<Used;i++)
    Buffer[i] = Buffer[i+1];
}

void sTextBuffer::Delete(sInt pos,sInt count)
{
  sVERIFY(pos>=0 && pos<Used);
  Used-=count;
  for(sInt i=pos;i<Used;i++)
    Buffer[i] = Buffer[i+count];
}

void sTextBuffer::Set(sInt pos,sChar c)
{
  sVERIFY(pos>=0 && pos<Used);
  Buffer[pos] = c;
}

/****************************************************************************/

void sTextBuffer::Indent(sInt count)
{
  sInt pos = Used;
  while(pos>0 && Buffer[pos]!='\n')
    pos--;
  sInt spaces = Used-pos;
  if(spaces<count)
    PrintF(L"%_",count-spaces);
}

void sTextBuffer::PrintChar(sInt c)
{
  Grow(1);
  Buffer[Used++] = c;
}

void sTextBuffer::Print(const sChar *text)
{
  Print(text,sGetStringLen(text));
}

void sTextBuffer::Print(const sChar *text,sInt c)
{
  Grow(c);
  sCopyMem(Buffer+Used,text,c*sizeof(sChar));
  Used+=c;
}

/****************************************************************************/
/***                                                                      ***/
/***   MemoryPool                                                         ***/
/***                                                                      ***/
/****************************************************************************/

sMemoryPool::sMemoryPoolBuffer::sMemoryPoolBuffer(sInt size)
{
  Start = new sU8[size];
  Current = Start;
  End = Start + size;
  Next = 0;
}

sMemoryPool::sMemoryPoolBuffer::~sMemoryPoolBuffer()
{
  delete[] Start;
}

/****************************************************************************/

sMemoryPool::sMemoryPool(sInt defaultsize)
{
  DefaultSize = defaultsize;
  First = Current = Last = new sMemoryPoolBuffer(DefaultSize);
}

sMemoryPool::~sMemoryPool()
{
  sMemoryPoolBuffer *i,*n;

  i = First;
  while(i)
  {
    n = i->Next;
    delete i;
    i = n;
  }
}

void sMemoryPool::Reset()
{
  sMemoryPoolBuffer *i,*n;

  i = First;
  while(i)
  {
    n = i->Next;
    delete i;
    i = n;
  }

  First = Current = Last = new sMemoryPoolBuffer(DefaultSize);
}

sU8 *sMemoryPool::Alloc(sInt size,sInt align)
{
  sU8 *result;
  if(size>DefaultSize/2)          // special case: very large object.
  {
    sMemoryPoolBuffer *b = new sMemoryPoolBuffer(size+align);
    b->Next = First;
    First = b;
    b->Current = sAlign(b->Current,align);
    result = b->Current;
    b->Current += size;
  }
  else                            // normal case: stuff it in!
  {
    Current->Current = sAlign(Current->Current,align);
    if(Current->Current+size>Current->End)
    {
      if(Current!=Last)
      {
        Current = Current->Next;
        Current->Current = sAlign(Current->Current,align);
      }
      else
      {
        Last->Next = new sMemoryPoolBuffer(DefaultSize);
        Last = Last->Next;
        Last->Current = sAlign(Last->Current,align);    // if align>DefaultSize/2 we have a problem...
        Current = Last;
      }
    }
    result = Current->Current;
    Current->Current += size;
  }

  return result;
}

void sMemoryPool::Push()
{
  sStackItem *item = Stack.AddMany(1);
  item->CBuffer = Current;
  item->CPtr = Current->Current;
}

void sMemoryPool::Pop()
{
  sInt count = Stack.GetCount();
  sVERIFY(count);
  sStackItem *item = &Stack[count-1];
  Current = item->CBuffer;
  sMemoryPoolBuffer *buf = item->CBuffer;
  buf->Current = item->CPtr;
  buf = buf->Next;
  while(buf)
  {
    buf->Current = buf->Start;
    buf = buf->Next;
  }
  Stack.RemTail();
}

sInt sMemoryPool::BytesAllocated() const
{
  const sMemoryPoolBuffer *buf = First;
  sInt allocsize = buf->Current-buf->Start;
  while(buf!=Last)
  {
    buf = buf->Next;
    allocsize += buf->Current-buf->Start;
  }
  return allocsize;
}

sInt sMemoryPool::BytesReserved() const
{
  const sMemoryPoolBuffer *buf = First;
  sInt memsize = buf->End-buf->Start;
  while(buf!=Last)
  {
    buf = buf->Next;
    memsize += buf->End-buf->Start;
  }
  return memsize;
}


/****************************************************************************/
/***                                                                      ***/
/***   StringPool                                                         ***/
/***                                                                      ***/
/****************************************************************************/

class sStringPool_
{  
  enum enums
  {
    HashSize = 4096,        // must be power of 2!
  };

  struct sStringPoolEntry
  {
    const sChar *Data;
    sStringPoolEntry *Next;
  };

  sMemoryPool *Strings;
  sMemoryPool *Entries;

  sStringPoolEntry *HashTable[HashSize];

public:
  sStringPool_()
  {
    Strings = new sMemoryPool();
    Entries = new sMemoryPool();
    sClear(HashTable);
  }

  ~sStringPool_()
  {
    delete Strings;
    delete Entries;
  }

  const sChar *Add(const sChar *s,sInt len, sBool *isnew)
  {
    sStringPoolEntry *e;
    sStringPoolEntry **hp;

    sU32 hash = sHashString(s,len);

    hp = &HashTable[hash & (HashSize-1)]; 
    e = *hp;
    while(e)
    {
      if(sCmpMem(e->Data,s,len*sizeof(sChar))==0 && e->Data[len]==0)
      {
        if (isnew) *isnew=sFALSE;
        return e->Data;
      }
      e = e->Next;
    }

    e = Entries->Alloc<sStringPoolEntry>();
    sChar *data = Strings->Alloc<sChar>(len+1);
    e->Data = data;
    sCopyMem(data,s,sizeof(sChar)*len);
    data[len] = 0;
    e->Next = *hp;
    *hp = e;

    if (isnew) *isnew=sTRUE;
    return e->Data;
  }
};

static sStringPool_ *sStringPool;
const sChar *sPoolStringEmpty;

const sChar *sAddToStringPool(const sChar *s,sInt len, sBool *isnew)
{
  return sStringPool->Add(s,len,isnew);
}

void sInitStringPool()
{
  if(!sStringPool)
  {
    sStringPool = new sStringPool_;
    sPoolStringEmpty = sAddToStringPool(L"",0);
  }
}

void sClearStringPool()
{
  sDelete(sStringPool);
}

sADDSUBSYSTEM(StringPool,0x11,sInitStringPool,sClearStringPool)

/****************************************************************************/
/***                                                                      ***/
/***   Rectangular Regions                                                ***/
/***                                                                      ***/
/****************************************************************************/

void sRectRegion::Clear()
{
  Rects.Clear();
}

void sRectRegion::Add(const sRect &add)
{
  Sub(add);
  Rects.AddTail(add);
}

void sRectRegion::Sub(const sRect &sub)
{
  sRect r;
  sInt i;

  for(i=0;i<Rects.GetCount();)
  {
    r = Rects[i];

    if(sub.IsInside(r))
    {
      Rects.RemAt(i);
      AddParts(r,sub);
    }
    else
    {
      i++;
    }
  }
}

void sRectRegion::AddParts(sRect old,const sRect &sub)
{
  sRect r;
  if(old.y1>sub.y0 && old.y0<sub.y0)      // something above?
  {
    r = old;
    r.y1 = sub.y0;
    Rects.AddTail(r);
    old.y0 = sub.y0;
  }
  if(old.y0<sub.y1 && old.y1>sub.y1)      // something below?
  {
    r = old;
    r.y0 = sub.y1;
    Rects.AddTail(r);
    old.y1 = sub.y1;
  }

  if(old.y0>=old.y1) return;              // is there something left?

  if(old.x1>sub.x0 && old.x0<sub.x0)      // something left?
  {
    r = old;
    r.x1 = sub.x0;
    Rects.AddTail(r);
    old.x0 = sub.x0;
  }
  if(old.x0<sub.x1 && old.x1>sub.x1)      // something right?
  {
    r = old;
    r.x0 = sub.x1;
    Rects.AddTail(r);
    old.x1 = sub.x1;
  }
}

/****************************************************************************/
/***                                                                      ***/
/***   String Map                                                         ***/
/***                                                                      ***/
/****************************************************************************/

sStringMap_::sStringMap_ (sInt numSlots)
{
  Size = numSlots;
  Slots = new Slot*[Size];
  sSetMem(Slots, 0, Size * sizeof(Slot*));
}

/****************************************************************************/

sStringMap_::~sStringMap_ ()
{
  Clear();
  sDeleteArray(Slots);
}

/****************************************************************************/

void sStringMap_::Clear ()
{
  for (sInt i = 0; i < Size; i++)
  {
    Slot * slot;
    while ((slot = Slots[i]))
    {
      Del(slot->key);
    }
  }
}

/****************************************************************************/

sU32 sStringMap_::Hash (const sChar * key) const
{
  sU32 hash = 0;

  for (sInt i = 0; key[i]; i++)
  {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash % Size;
}

/****************************************************************************/

void sStringMap_::Del (const sChar * key)
{
  sInt slotNum = Hash(key);
  Slot * slot = Slots[slotNum];
  Slot * last = sNULL;

  if (slot)
  {
    while (1)
    {
      if (sCmpString(slot->key, key) == 0)
      {
        if (slot->next)
        {
          if (last) last->next = slot->next;
          else Slots[slotNum] = slot->next;
        }
        else
        {
          if (last) last->next = sNULL;
          else Slots[slotNum] = slot->next;
        }

        delete[] slot->key;
        sDelete(slot);
        return;
      }
      if (!slot->next) return; // key not found
      last = slot;
      slot = slot->next;
    }
  }
}

/****************************************************************************/

void * sStringMap_::Get (const sChar * key) const
{
  sInt slotNum = Hash(key);
  Slot * slot = Slots[slotNum];
  if (slot)
  {
    while (1)
    {
      if (sCmpString(slot->key, key) == 0)
        return slot->value;
      if (!slot->next) break; // key not found
      slot = slot->next;
    }
  }

  return sNULL;
}

/****************************************************************************/

void sStringMap_::Set (const sChar * key, void * value)
{
  sInt slotNum = Hash(key);

  Slot * slot = Slots[slotNum];
  if (slot)
  {
    while (1)
    {
      if (sCmpString(slot->key, key) == 0)
      { 
        sDelete (slot->key);
        break; // key already inserted
      }
      else if (!slot->next)
      {
        slot->next = new Slot; 
        slot = slot->next; 
        slot->next = sNULL;
        break;
      }

      slot = slot->next;
    }
  }
  else
  {
    Slots[slotNum] = slot = new Slot;
    slot->next = sNULL;
  }

  slot->value = value;  
  sInt keyLen = sGetStringLen(key)+1;
  slot->key = new sChar[keyLen];
  sCopyString(slot->key, key, keyLen);
}

/****************************************************************************/

sInt sStringMap_::GetCount () const
{
  sInt count = 0;
  for (sInt slotNum = 0; slotNum < Size; slotNum++)
  {
    if (Slots[slotNum])
    {
      count++;
      Slot * slot = Slots[slotNum];
      while ((slot = slot->next)) count++;
    }
  }
  return count;
}

/****************************************************************************/

sStaticArray<sChar *> * sStringMap_::GetKeys () const
{
  sStaticArray<sChar *> * keys = new sStaticArray<sChar *>(GetCount());
  for (sInt slotNum = 0; slotNum < Size; slotNum++)
  {
    if (Slots[slotNum])
    {
      Slot * slot = Slots[slotNum];
      keys->AddTail(slot->key);
      while ((slot = slot->next)) keys->AddTail(slot->key);
    }
  }
  return keys;
}

/****************************************************************************/

void sStringMap_::Dump () const
{
  for (sInt i = 0; i < Size; i++)
  {
    Slot * slot = Slots[i];
    sPrintF(L"%02d: ", i);
    if (slot)
    {
      while (1)
      {
        sPrintF(L"%s->%x ", slot->key, (sDInt)slot->value);

        if (!slot->next) 
        {
          sPrintF(L"\n");
          break;
        }

        slot = slot->next;
      }
    }
    else
    {
      sPrintF(L"\n");
    }
  }
}

/****************************************************************************/
/***                                                                      ***/
/*** stream classes                                                       ***/
/***                                                                      ***/
/****************************************************************************/
/*
sStreamMemory::sStreamMemory()
{
  Ptr=0;
}

sStreamMemory::sStreamMemory(sDInt size)
{
  Ptr=0;
  Open(size);
  sVERIFY(Ptr);
}

sStreamMemory::sStreamMemory(const void *buffer, sDInt size, sBool iamowner)
{
  Ptr=0;
  Open(buffer,size,iamowner);
  sVERIFY(Ptr);
}

sStreamMemory::sStreamMemory(void *buffer,sDInt size, sBool iamowner)
{
  Ptr=0;
  Open(buffer,size,iamowner);  
  sVERIFY(Ptr);
}

sStreamMemory::sStreamMemory(sStream &str)
{
  Ptr=0;
  Open(str);
  sVERIFY(Ptr);
}


sBool sStreamMemory::Open(sDInt size)
{
  Close();
  sVERIFY(size>=0);
  if (size<=0) return sFALSE;
  BSize = size;
  Pos = 0;
  Owner = sFALSE;
  Readonly=sFALSE;
  Ptr = new sU8[size];
  return Ptr!=0;
}
 

sBool sStreamMemory::Open(const void *buffer, sDInt size, sBool iamowner)
{
  Close();
  Ptr=reinterpret_cast<sU8*>(const_cast<void*>(buffer));
  BSize=size;
  Owner=iamowner;
  Readonly=sTRUE;
  Pos=0;
  return Ptr!=0;
}
 

sBool sStreamMemory::Open(void *buffer, sDInt size, sBool iamowner)
{
  Close();
  Ptr=reinterpret_cast<sU8*>(buffer);
  BSize=size;
  Owner=iamowner;
  Readonly=sFALSE;
  Pos=0;
  return Ptr!=0;
}
 

sBool sStreamMemory::Open(sStream &str)
{
  Close();
  sVERIFY(str.CheckFlags(sSF_READ|sSF_STS));
  if (!str.CheckFlags(sSF_READ|sSF_STS))
    return sFALSE;
  BSize=str.Size()-str.Tell();
  Readonly=sFALSE;
  Owner=sTRUE;
  Pos=0;
  Ptr=new sU8[BSize];
  return (Ptr!=0 && str.ReadB(Ptr,BSize));
}


sStreamMemory::~sStreamMemory()
{
  Close();
}

sInt  sStreamMemory::GetFlags()
{
  sInt f=0;
  if (Ptr)
  {
    f|=sSF_READ|sSF_STS|sSF_CLONABLE|sSF_HASPTR;
    if (!Readonly)
      f|=sSF_WRITE;
  }
  return f;
}

sBool  sStreamMemory::Close()
{
  if (Ptr)
  {
    if (Owner)
      sDelete(Ptr);
    Ptr=0;
  }
  return sTRUE;
}

sDInt sStreamMemory::Read(void *ptr, sDInt count)
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  count=sMin(count,BSize-Pos);
  if (count) 
    sCopyMem(ptr,Ptr+Pos,count);
  Pos+=count;
  return count;
}

sDInt sStreamMemory::Write(const void *ptr, sDInt count)
{
  sVERIFY(Ptr && !Readonly);
  if (!Ptr || Readonly) return 0;
  count=sMin(count,BSize-Pos);
  if (count) 
    sCopyMem(Ptr+Pos,ptr,count);
  Pos+=count;
  return count;
}

sSize sStreamMemory::Seek(sSize pos)
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  Pos=sClamp<sSize>(pos,0,BSize);
  return Pos;
}

sSize sStreamMemory::SeekEnd(sSize offset)
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  Pos=sClamp<sSize>(BSize+offset,0,BSize);
  return Pos;
}

sSize sStreamMemory::SeekCur(sSize offset)
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  Pos=sClamp<sSize>(Pos+offset,0,BSize);
  return Pos;
}

sBool sStreamMemory::EOF_()
{
  return !Ptr || Pos==BSize;
}

sSize sStreamMemory::Tell()
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  return Pos;
}

sSize sStreamMemory::Size()
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  return BSize;
}

const void *sStreamMemory::GetPtr(sDInt &count)
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  count=sMin(count,BSize-Pos);
  const void *ret=Ptr+Pos;
  Pos+=count;
  return ret;
}

void  *sStreamMemory::Detach(sDInt *size)
{
  sVERIFY(Ptr);
  if (!Ptr || !Owner) return 0;
  if (size) *size=BSize;
  void *ret=Ptr;
  Ptr=0;
  return ret;
}

sStream *sStreamMemory::Clone()
{
  sVERIFY(Ptr);
  if (!Ptr) return 0;
  sStreamMemory *str=new sStreamMemory;
  str->Open(Ptr,BSize,sFALSE);
  str->Pos=Pos;
  return str;
}
*/
/****************************************************************************/
/*
sBool sStreamPart::Open(sStream &str, sSize from, sSize size, sBool clone)
{
  Close();
  sBool sf=str.CheckFlags(sSF_STS);
  sVERIFY(sf);
  if (!sf) return sFALSE;

  if (clone)
  {
    sf=str.CheckFlags(sSF_CLONABLE);
    sVERIFY(sf);
    if (!sf) return sFALSE;
    Str=str.Clone();
    if (!Str) return sFALSE;
    Owner=sTRUE;
  }
  else
  {
    Str=&str;
    Owner=sFALSE;
  }

  sSize rsize=Str->Size();
  PPos=sClamp<sSize>(from,0,rsize);
  PSize=sMin(PSize,rsize-PPos);
  Pos=0;
  return sTRUE;
}

sStream *Str;
sSize   PPos, PSize, Pos;
sBool   Owner;

sStreamPart::sStreamPart()
{
  Str=0;
}

sStreamPart::sStreamPart(sStream &str, sSize from, sSize size, sBool clone)
{
  Str=0;
  Open(str,from,size,clone);
  sVERIFY(Str);
}


sStreamPart::~sStreamPart()
{
  Close();
}

sInt  sStreamPart::GetFlags()
{
  sInt f=0;
  if (Str)
  {
    f|=Str->GetFlags()&(sSF_READ|sSF_WRITE);
    f|=sSF_STS|sSF_CLONABLE;
  }
  return f;
}

sBool sStreamPart::Close()
{
  if (Str)
  {
    if (Owner)
      sDelete(Str);
    Str=0;
  }
  return sTRUE;
}

sDInt sStreamPart::Read(void *ptr, sDInt count)
{
  sVERIFY(Str);
  if (!Str) return 0;
  Str->Seek(PPos+Pos);
  sDInt read=Str->Read(ptr,count);
  Pos+=read;
  return read;
}

sDInt sStreamPart::Write(const void *ptr, sDInt count)
{
  sVERIFY(Str);
  if (!Str) return 0;
  Str->Seek(PPos+Pos);
  sDInt written=Str->Write(ptr,count);
  Pos+=written;
  return written;
}

sSize sStreamPart::Seek(sSize pos)
{
  sVERIFY(Str);
  if (!Str) return 0;
  Pos=sClamp<sSize>(pos,0,PSize);
  return Pos;
}

sSize sStreamPart::SeekCur(sSize offs)
{
  sVERIFY(Str);
  if (!Str) return 0;
  Pos=sClamp<sSize>(Pos+offs,0,PSize);
  return Pos;
}

sSize sStreamPart::SeekEnd(sSize offs)
{
  sVERIFY(Str);
  if (!Str) return 0;
  Pos=sClamp<sSize>(PSize+offs,0,PSize);
  return Pos;
}

sBool sStreamPart::EOF_()
{
  return Str==0 || Pos==PSize;
}

sSize sStreamPart::Tell()
{
  sVERIFY(Str);
  if (!Str) return 0;
  return Pos;
}

sSize sStreamPart::Size()
{
  sVERIFY(Str);
  if (!Str) return 0;
  return PSize;
}

sStream *sStreamPart::Clone()
{
  sVERIFY(Str);
  if (!Str) return 0;
  sStreamPart *sp=new sStreamPart;
  sp->Str=Str;
  sp->Owner=0;
  sp->Pos=0;
  sp->PPos=PPos;
  sp->PSize=PSize;
  return sp;
}
*/
/****************************************************************************/
/*
sBool sStreamCache::Open(sStream &str, sDInt granule)
{
  Close();
  Str=&str;
  WBufSize=0;
  RBufPos=0;
  RBufSize=0;
  WBuf=0;
  RBuf=0;
  BufSize=granule;
  Flags=Str->GetFlags()&(sSF_READ|sSF_WRITE|sSF_ASYNC);
  if (Flags&sSF_READ)
  {
    RBuf = new sU8[BufSize];
    if (!RBuf)
      return sFALSE;
  }
  if (Flags&sSF_WRITE)
  {
    WBuf = new sU8[BufSize];
    if (!WBuf)
      return sFALSE;
  }
  return sTRUE;
}

sStreamCache::sStreamCache()
{
  Str=0;
  WBuf=0;
  RBuf=0;
}

sStreamCache::sStreamCache(sStream &str, sDInt granule)
{
  Str=0;
  WBuf=0;
  RBuf=0;
  Open(str,granule);
  sVERIFY(Str);
}


// sStream impl
sStreamCache::~sStreamCache()
{
  Close();
}

void sStreamCache::Flush()
{
  if (Str && WBuf && WBufSize)
  {
    sSize written=Str->Write(WBuf,WBufSize);
    sVERIFY(written==WBufSize);
    WBufSize=0;
  }
}

sInt sStreamCache::GetFlags()
{
  return Str?Flags:0;
}

sBool sStreamCache::Close()
{
  if (Str)
  {
    Flush();
    sDelete(WBuf);
    sDelete(RBuf);
    Str=0;
  }
  return sTRUE;
}

sDInt sStreamCache::Read(void *ptr, sDInt count)
{
  sVERIFY(Str && (Flags&sSF_READ));
  if (!Str || !(Flags&sSF_READ))
    return 0;
  sU8 * const op=reinterpret_cast<sU8*>(ptr);
  sU8  *dp=op;
  while (count>0)
  {
    sDInt todo=sMin(count,RBufSize-RBufPos);
    if (todo) // copy part of buffer
    {
      sCopyMem(dp,RBuf+RBufPos,todo);
      dp+=todo;
      RBufPos+=todo;
      count-=todo;
    }
    if (RBufPos==RBufSize)
    {
      Flush();
      RBufPos=0;
      RBufSize=Str->Read(RBuf,BufSize);
      if (!RBufSize) // empty? eof? error?
        break;
    }
  }
  return dp-op;
}


sDInt sStreamCache::Write(const void *ptr, sDInt count)
{
  sVERIFY(Str && (Flags&sSF_READ));
  if (!Str || !(Flags&sSF_READ))
    return 0;
  sDInt c2=count;
  const sU8 *sp=reinterpret_cast<const sU8*>(ptr);
  while (c2>0)
  {
    if (WBufSize==BufSize)
      Flush();
    sDInt todo=sMin(c2,BufSize-WBufSize);
    sCopyMem(WBuf+WBufSize,sp,todo);
    WBufSize+=todo;
    sp+=todo;
    c2-=todo;
  }
  return count;
}


sBool sStreamCache::EOF_()
{
  return (Str==0 || (RBufPos==RBufSize && Str->EOF_()));
}
*/


/****************************************************************************/
/*

sStreamTemp::sStreamTemp(sBool open)
{
  if (open)
  {
    Open();
    sVERIFY(!Nodes.IsEmpty());
  }
}

sBool sStreamTemp::Open()
{
  Close();
  Buffer *b=new Buffer;
  if (!b) return sFALSE;
  Nodes.AddTail(b);
  FSize=0;
  FPos=0;
  CurB=b;
  BPos=0;
  return sTRUE;
}


// sStream impl
sStreamTemp::~sStreamTemp()
{
  Close();
}

sInt  sStreamTemp::GetFlags()
{
  if (Nodes.IsEmpty())
    return 0;
  else
    return sSF_READ|sSF_WRITE|sSF_STS|sSF_CLONABLE;
}

sBool sStreamTemp::Close()
{
  while (!Nodes.IsEmpty())
  {
    Buffer *bla=Nodes.GetHead();
    Nodes.RemHead();
    sDelete(bla);
  }
  return sTRUE;
}

sDInt sStreamTemp::Read(void *ptr, sDInt count)
{
  sVERIFY(!Nodes.IsEmpty());
  if (Nodes.IsEmpty())
    return 0;
  count=sMin(count,FSize-FPos);

  sDInt c2=count;
  sU8 *dp=reinterpret_cast<sU8*>(ptr);
  while (c2)
  {
    sDInt todo=sMin(c2,BUFSIZE-BPos);
    if (todo)
    {
      sCopyMem(dp,CurB->Mem+BPos,todo);
      BPos+=todo;
      dp+=todo;
      c2-=todo;
    }
    else
    {
      BPos=0;
      CurB=Nodes.GetNext(CurB);
    }
  }
  FPos+=count;
  return count;
}

sDInt sStreamTemp::Write(const void *ptr, sDInt count)
{
  sVERIFY(!Nodes.IsEmpty());
  if (Nodes.IsEmpty())
    return 0;
  const sU8 *sp=reinterpret_cast<const sU8*>(ptr);
  const sU8 * const op=sp;
  while (count)
  {
    sDInt todo=sMin(count,BUFSIZE-BPos);
    if (todo)
    {
      sCopyMem(CurB->Mem+BPos,sp,todo);
      BPos+=todo;
      FPos+=todo;
      sp+=todo;
      count-=todo;
      if (FPos>FSize) FSize=FPos;
    }
    else
    {
      if (CurB == Nodes.GetTail())
        Nodes.AddTail(new Buffer);
      BPos=0;
      CurB = Nodes.GetNext(CurB);
    }
  }
  return sp-op;
}

sSize sStreamTemp::Seek(sSize pos)
{
  sVERIFY(!Nodes.IsEmpty());
  if (Nodes.IsEmpty())
    return 0;
  sDInt offs=sClamp<sSize>(pos,0,FSize)-FPos;

  FPos+=offs;
  BPos+=offs;

  while (BPos>BUFSIZE)
  {
    CurB=Nodes.GetNext(CurB);
    BPos-=BUFSIZE;
  }
  while (BPos<0)
  {
    CurB=Nodes.GetPrev(CurB);
    BPos+=BUFSIZE;
  }

  return FPos;
}

sSize sStreamTemp::Tell()
{
  sVERIFY(!Nodes.IsEmpty());
  if (Nodes.IsEmpty())
    return 0;
  return FPos;
}

sSize sStreamTemp::Size()
{
  sVERIFY(!Nodes.IsEmpty());
  if (Nodes.IsEmpty())
    return 0;
  return FSize;
}

sBool sStreamTemp::EOF_()
{
  return Nodes.IsEmpty() || FPos==FSize;
}

sStream *sStreamTemp::Clone()
{
  sVERIFY(!Nodes.IsEmpty());
  if (Nodes.IsEmpty())
    return 0;

  sStreamTemp *st=new sStreamTemp;
  st->FSize=FSize;
  st->FPos=FPos;
  st->BPos=BPos;

  // full copy (perhaps we could have weak semantics later)
  Buffer *b;
  sFORALL_LIST(Nodes,b)
  {
    Buffer *nb=new Buffer;
    sCopyMem(nb->Mem,b->Mem,BUFSIZE);
    st->Nodes.AddTail(nb);
    if (b==CurB) st->CurB=nb;
  }

  return st;
}

*/

/****************************************************************************/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/****************************************************************************/
