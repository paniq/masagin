/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   This source contains all basic classes that require dynamic        ***/
/***   memory management.                                                 ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef FILE_BASE_TYPES2_HPP
#define FILE_BASE_TYPES2_HPP

#include "base/types.hpp"
#include "base/serialize.hpp"

/****************************************************************************/
/***                                                                      ***/
/***   Containers                                                         ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   sArray is an automatically resizing version of sStaticArray.       ***/
/***                                                                      ***/
/****************************************************************************/

template <class Type> class sArray : public sStaticArray<Type>
{
  void ReAlloc(sInt max)          { if(max>=this->Used && max!=this->Alloc) { Type *n=new Type[max]; 
                                    for(sInt i=0;i<this->Used;i++) n[i]=this->Data[i]; 
                                    delete[] this->Data; this->Data=n; this->Alloc=max; } }
  void Grow(sInt add)             { if(this->Used+add>this->Alloc) ReAlloc(sMax(this->Used+add,this->Alloc*2)); }
};

template <class Type> class sAutoArray : public sArray<Type>
{
public:
  ~sAutoArray<Type>() { sDeleteAll(*this); }
};

/****************************************************************************/
/***                                                                      ***/
/***   Garbage Collection Memory Management                               ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   for each GC class, implement the Tag() member so that it calls     ***/
/***   Need() for every sObject-derived member. You don't have to care    ***/
/***   about cyclic graphs, that's all handled for you in Need().         ***/
/***   Need() may be called with a 0 pointer.                             ***/
/***                                                                      ***/
/***   To get some runtime class identification, you should add the       ***/
/***   macro sCLASSNAME(MyClass).                                          ***/
/***                                                                      ***/
/***   To Check for a certain class, you don't need to compare strings,   ***/
/***   You can compare the pointer with the one returned by the static    ***/
/***   function, like: if(o->GetClassName()==sButtonControl::ClassName()) ***/
/***                                                                      ***/
/***   You will need to register and unregister the Root of your object   ***/
/***   model with sAddRoot() / sRemRoot(). You can register as many       ***/
/***   roots as you like. make sure that every object in an object model  ***/
/***   is reached through the Tag() recursion from the root object.       ***/
/***                                                                      ***/
/***   The garbage collector is triggered with the sCollect() function.   ***/
/***   The collection takes place after processing for this frame has     ***/
/***   ended. It is save to trigger the collection multiple times per     ***/
/***   frame, it will not happen more than once per frame. Usually it     ***/
/***   is fast enough to do the collection every frame.                   ***/
/***                                                                      ***/
/***   The finalize phase solves a problem in the destructor: when the    ***/
/***   destructor is called, some childs may have already be deleted.     ***/
/***   So often important cleanup work can not be done. The finally       ***/
/***   phase is called for all objects pending for deletion before any    ***/
/***   destructor is called. In this phase you should not perform         ***/
/***   destruction, just some cleanup, like working with nonmanaged       ***/
/***   objects or stopping helper threads. After finalizing, all managed  ***/
/***   objects must be in a valid state.                                  ***/
/***                                                                      ***/
/***   For multithreaded applications you have to lock the garbage        ***/
/***   collector with sGCLock() when working on gargabe collected         ***/
/***   structures with creating new garbage collected objects.            ***/
/***                                                                      ***/
/****************************************************************************/

class sObject
{
public:

  // implement these:

  virtual void Tag() {}

  // internal functions:

  sObject();                      // adds the object to GC
  virtual ~sObject();
  static const sChar *ClassName() { return L"sObject"; }
  static sObject *NewObjectStatic() { return new sObject; }
  virtual sObject *NewObjectVirtual() { return new sObject; }
  virtual const sChar *GetClassName() { return ClassName(); }
  virtual void Finalize();        // finalize phase: all objects are still valid, but are soon deleted.
  sInt NeedFlag;                  // flag needed objects
  void Need();                    // this implements the GC recursion
};

// use this macro to implement the classname thing...

#define sCLASSNAME(name) \
static const sChar *ClassName() { return L ## #name; } \
static sObject *NewObjectStatic() { return new name; } \
sObject *NewObjectVirtual() { return new name; } \
const sChar *GetClassName() { return ClassName(); }

#define sCLASSNAME_NONEW(name) \
static const sChar *ClassName() { return L ## #name; } \
const sChar *GetClassName() { return ClassName(); }

/****************************************************************************/

void sAddRoot(sObject *);         // add an object as GC root
void sRemRoot(sObject *);         // add an object as GC root
void sCollect();                  // do the recursion
void sGCLock();                   // lock/unlock for multithreaded GC use
void sGCUnlock();

/****************************************************************************/

template <template <typename> class ArrayType,class BaseType> 
void sNeed(ArrayType<BaseType> &a)
{  
  BaseType o;
  sFORALL(a,o) o->Need();
}

/****************************************************************************/
/***                                                                      ***/
/***   A message system for sObject                                       ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   Most usefull for the GUI.                                          ***/
/***                                                                      ***/
/***   A message has                                                      ***/
/***                                                                      ***/
/***   - a target object                                                  ***/
/***   - a target member function                                         ***/
/***   - an sDInt code                                                    ***/
/***                                                                      ***/
/***   message system features                                            ***/
/***                                                                      ***/
/***   - Send messages immediatly                                         ***/
/***   - Post messages for later execution                                ***/
/***   - message are copied when posting, so the original may die.        ***/
/***   - it is safe to call Send or Post with 0-this-ptr and 0-target-ptr ***/
/***                                                                      ***/
/****************************************************************************/

struct sMessage
{
  sInt Type;
  sDInt Code;
  sObject *Target;
  union
  {
    void (sObject::*Func1)();
    void (sObject::*Func2)(sDInt);
    void (         *Func3)(sObject *);
    void (         *Func4)(sObject *,sDInt);
    void (sObject::*Func11)(const struct sWindowDrag &);
    void (sObject::*Func12)(const struct sWindowDrag &,sDInt);
  };

  sMessage()                                                                                      { Type=0; Code=0; Target=0; Func1=0; }
//  sMessage(const sMessage &m)                                                                     { Type=m.Type; Code=m.Code; Target=m.Target; Func1=m.Func1; }
  template<class T, class TBase> sMessage(T *target,void (TBase::*func)()                                ,sDInt code=0) { Type=1;    Code=code; Target=static_cast<TBase*>(target); Func1 =(void (sObject::*)()                )func; }
  template<class T, class TBase> sMessage(T *target,void (TBase::*func)(sDInt)                           ,sDInt code=0) { Type=2;    Code=code; Target=static_cast<TBase*>(target); Func2 =(void (sObject::*)(sDInt)           )func; }
                           sMessage(sObject *target,void (*func)(sObject*)                               ,sDInt code=0) { Type=3;    Code=code; Target=(sObject *)         target ; Func3 =func; }
                           sMessage(sObject *target,void (*func)(sObject*,sDInt)                         ,sDInt code=0) { Type=4;    Code=code; Target=(sObject *)         target ; Func4 =func; }
  template<class T, class TBase> sMessage(T *target,void (TBase::*func)(const struct sWindowDrag &      ),sDInt code=0) { Type=0x11; Code=code; Target=static_cast<TBase*>(target); Func11=(void (sObject::*)(const struct sWindowDrag &      ))func; }
  template<class T, class TBase> sMessage(T *target,void (TBase::*func)(const struct sWindowDrag &,sDInt),sDInt code=0) { Type=0x12; Code=code; Target=static_cast<TBase*>(target); Func12=(void (sObject::*)(const struct sWindowDrag &,sDInt))func; }

  void Post() const;                      // enque message
  void PostASync() const;                 // thread save message posting
  void Send() const;                      // send now!
  void Drag(const struct sWindowDrag &) const;  // misuse message to send dragging. can't post dragging :-)
  static void Pump();                     // send enqued messages
  sBool IsEmpty() { return Type==0; }     // true for unset message
  sBool IsValid() { return Type!=0; }     // true for set message
  void Clear() { Type=0; Target=0; Code=0; Func1=0; }
};

/****************************************************************************/
/***                                                                      ***/
/***   Text Buffer Class (dynamic string of some kind)                    ***/
/***                                                                      ***/
/***   Recomended use:                                                    ***/
/***   - Preparing output for a text file, logging.                       ***/
/***   - As buffer for a text editor control                              ***/
/***   - for potentially long strings of variable size                    ***/
/***                                                                      ***/
/***   consider sPoolString for non-changing strings                      ***/
/***   consider sString<> for small strings (64 characters or less)       ***/
/***                                                                      ***/
/****************************************************************************/

class sTextBuffer
{
  sInt Alloc;                     // allocated character count
  sInt Used;                      // used character count
  sChar *Buffer;                  // buffer. not alway 0-terminated
  void Grow(sInt add);            // grow to make space for some additional chars
  void Init();
public:
  sTextBuffer();
  sTextBuffer(const sTextBuffer& tb);
  sTextBuffer(const sChar* t);
  ~sTextBuffer();

  sTextBuffer& operator=(const sTextBuffer& tb);
  sTextBuffer& operator=(const sChar* t);

  // serialization
  template <class streamer> void Serialize_(streamer &);
  void Serialize(sReader &stream);
  void Serialize(sWriter &stream);

  // buffer management

  void Clear();                   // clear buffer. don't deallocate
  void SetSize(sInt count);       // enlarge and preallocate buffer
  const sChar *Get() const;       // terminate buffer and return pointer !!! THIS IS NOT CONST !!!
  sChar *Get();                   // terminate buffer and return pointer
  sInt GetCount() { return Used; };
  void Insert(sInt pos,sChar c);
  void Insert(sInt pos,const sChar *c,sInt len=-1);
  void Set(sInt pos,sChar c);
  void Delete(sInt pos);
  void Delete(sInt pos,sInt count);

  // printing. this appends!

  void Indent(sInt count);        // finds last linefeed and indents with spaces
  void PrintChar(sInt c);
  void Print(const sChar *text);
  void Print(const sChar *text,sInt len);

  sPRINTING0(PrintF, sString<1024> tmp; sFormatStringBuffer buf=sFormatStringBase(tmp,format);buf,Print((const sChar*)tmp););
};


/****************************************************************************/
/***                                                                      ***/
/***   MemoryPool                                                         ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   This is a pool from where you can cheaply allocate small objects.  ***/
/***   - unlike an array you can not enumerate them                       ***/
/***   + it grows more efficiently than an array                          ***/
/***   + it is more efficient in time and space than many new's           ***/
/***   - you can only delete all objects in one flush                     ***/
/***   - constructors / destructors are not called                        ***/
/***   + everything will be correctly aligned                             ***/
/***   + minimal template usage                                           ***/
/***                                                                      ***/
/****************************************************************************/

class sMemoryPool
{
  sInt DefaultSize;
  struct sMemoryPoolBuffer
  {
    sU8 *Start;
    sU8 *Current;
    sU8 *End;
    sMemoryPoolBuffer *Next;
    sMemoryPoolBuffer(sInt size);
    ~sMemoryPoolBuffer();
  };
  sMemoryPoolBuffer *Current;
  sMemoryPoolBuffer *First;
  sMemoryPoolBuffer *Last;

  struct sStackItem
  {
    sMemoryPoolBuffer *CBuffer;
    sU8 *CPtr;
  };
  sArray<sStackItem> Stack;
public:

  sMemoryPool(sInt defaultsize = 65536);
  ~sMemoryPool();
  sU8 *Alloc(sInt size,sInt align);

  sInt BytesAllocated() const;
  sInt BytesReserved() const;

  void Reset();     // frees all reserved memory after the first default size block
  void Push();      // pushes the current allocation cfg to internal stack
  void Pop();       // everything allocated after last push is freed

  template <typename T> T *Alloc(sInt n=1) { return (T*) Alloc(sizeof(T)*n,sALIGNOF(T)); }
};


/****************************************************************************/
/***                                                                      ***/
/***   sPoolArray                                                         ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   useful for constructing large arrays without knowing their final   ***/
/***   size. grows without reallocation. pointers stay valid.             ***/
/***                                                                      ***/
/***   similar too sMemoryPool but:                                       ***/
/***                                                                      ***/
/***   * managing only items of one data type                             ***/
/***   + items can be enumerated                                          ***/
/***                                                                      ***/
/****************************************************************************/

template <typename T> class sPoolArray
{
  struct PoolItem
  {
    PoolItem(sInt size)                 { Data = new T[size]; }
    ~PoolItem()                         { sDeleteArray(Data); }
    T *Data;
  };

  sArray<PoolItem*> Items;
  sInt Used;
  sInt Alloc;
  sInt Log2Size;

  sInt ItemSize;
  sInt Mask;

  void Grow()                           { Items.AddTail(new PoolItem(ItemSize)); Alloc += ItemSize; }

  const T& GetUnsafe(sInt i) const      { return Items[i>>Log2Size]->Data[i&Mask]; }
  T& GetUnsafe(sInt i)                  { return Items[i>>Log2Size]->Data[i&Mask]; }
public:

  sPoolArray(sInt l2s=12):Used(0),Alloc(0),Log2Size(l2s) { ItemSize = 1<<Log2Size; Mask = ItemSize-1; }
  ~sPoolArray()                         { Reset(); }

  void Clear()                          { Used = 0; }
  void Reset(sInt l2s=-1)               { sDeleteAll(Items); Alloc = 0; Used = 0; if(l2s!=-1) {Log2Size=l2s; ItemSize = 1<<l2s; Mask = ItemSize-1; } }

  sInt GetCount() const                 { return Used; }
  sInt GetItemCount() const             { return Items.GetCount(); }
  sInt GetItemSize() const              { return ItemSize; }
  sBool IsEmpty() const                 { return Used==0; }
  sBool IsFull() const                  { return Used==Alloc; }

  void AddTail(const T &e)              { if(Used>=Alloc) Grow(); GetUnsafe(Used) = e; Used++; }
  T* AddTail()                          { if(Used>=Alloc) Grow(); T *ptr = &(GetUnsafe(Used)); Used++; return ptr; }
  T* AddMany(sInt &count)               { count = (ItemSize-(Used&Mask)); if(Used==Alloc) Grow(); T *ptr = &(GetUnsafe(Used)); Used+=count; return ptr;}

  const T& operator[](sInt i) const     { sVERIFY(i>=0 && i<=Used); return Items[i>>Log2Size]->Data[i&Mask]; }
  T& operator[](sInt i)                 { sVERIFY(i>=0 && i<=Used); return Items[i>>Log2Size]->Data[i&Mask]; }


  void RemAt(sInt p)                    { (*this)[p] = (*this)[Used-1]; Used--; }
  void RemTail()                        { sVERIFY(Used); Used--; }

  void Swap(sPoolArray &a)              { sSwap(Used,a.Used); sSwap(Alloc,a.Alloc); sSwap(Log2Size,a.Log2Size);
                                          sSwap(ItemSize,a.ItemSize); sSwap(Mask,a.Mask); sSwap(Items,a.Items); }
  void ConvertTo(sStaticArray<T> &dest) const;
};

/****************************************************************************/

template <typename T> 
void sPoolArray<T>::ConvertTo(sStaticArray<T> &dest) const
{
  dest.HintSize(Used);
  sInt left = Used;
  for(sInt i=0;i<Items.GetCount();i++)
  {
    T *dst = dest.AddMany(sMin(ItemSize,left));
    const T *src = Items[i]->Data;
    for(sInt k=0;k<ItemSize && k<left;k++)
      *dst++ = *src++;
    left -= ItemSize;
  }
}

/****************************************************************************/
/***                                                                      ***/
/***   StringPool of PoolStrings                                          ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   If you don't want to care about string memory management,          ***/
/***   you can add a string to a stringpool and receive a handle.         ***/
/***   - the memory is never freed                                        ***/
/***   - same strings will only be stored once                            ***/
/***   - same strings will receive the same handle                        ***/
/***   - strings can not be altered                                       ***/
/***   this is perfect for applications like parsers and symbol tables.   ***/
/***   this is awful when the strings get changed often, and              ***/
/***   catastrophic for a text editor.                                    ***/
/***                                                                      ***/
/***   there is an sClearStringPool(), but it is only valid to call       ***/
/***   when you are absolutly sure that ALL sPoolStrings are destroyed!   ***/
/***                                                                      ***/
/***   the stringpool itself is a global variable. it makes little        ***/
/***   sense to have multiple stringpools.                                ***/
/***                                                                      ***/
/****************************************************************************/

const sChar *sAddToStringPool(const sChar *s,sInt len, sBool *isnew=0);
void sClearStringPool();       
extern const sChar *sPoolStringEmpty;

struct sPoolString              // a string from the pool
{
private:
  const sChar *Buffer;
public:
  sPoolString()                               { Buffer = sPoolStringEmpty; }
  sPoolString(const sChar *s)                 { Buffer = sAddToStringPool(s,sGetStringLen(s)); }
  sPoolString(const sChar *s,sInt len)        { Buffer = sAddToStringPool(s,len); }
  sPoolString &operator=(const sChar *s)      { Buffer = sAddToStringPool(s,sGetStringLen(s)); return *this; }
  void Init(const sChar *s)                   { Buffer = sAddToStringPool(s,sGetStringLen(s)); }
  void Init(const sChar *s,sInt len)          { Buffer = sAddToStringPool(s,len); }

  operator const sChar*() const               { return Buffer; }
  const sChar *Get() const                    { return Buffer; }


  sInt operator==(const sChar *s) const       { return sCmpString(Buffer,s)==0; }
  sInt operator==(const sPoolString &s) const { return Buffer==s.Buffer; }
  sInt operator!=(const sChar *s) const       { return sCmpString(Buffer,s)!=0; }
  sInt operator!=(const sPoolString &s) const { return Buffer!=s.Buffer; }
  sChar operator[](sInt i) const              { return Buffer[i]; }
  sInt Count() const                          { return sGetStringLen(Buffer); }
  sBool IsEmpty() const                       { return Buffer[0]==0; }
  sU32 GetHash() const                        { return sHashString(Buffer); }
};

inline sReader& operator| (sReader &s,sPoolString &a) { sInt len; s | len; a.Init( (const sChar *) s.GetPtr(len*sizeof(sChar)) ,len); s.Align(4); return s; }

/****************************************************************************/
/***                                                                      ***/
/***   Rectangular Regions                                                ***/
/***                                                                      ***/
/****************************************************************************/

class sRectRegion
{
  void AddParts(sRect old,const sRect &sub);
public:
  sArray<sRect> Rects;
  void Clear();
  void Add(const sRect &r);
  void Sub(const sRect &r);
};

/****************************************************************************/
/***                                                                      ***/
/***   "Automatic" linked list of all objects of a certain type           ***/
/***   to be used as a base class                                         ***/
/***                                                                      ***/
/****************************************************************************/
/*
template <class T> class sChain
{
  static sChain<T> *_Root;
  sChain<T> *_Next;
  sChain<T> *_Prev;

public:

  static inline T* GetRoot() { return static_cast<T*>(_Root); }
  inline T* GetNext() { return static_cast<T*>(_Next); }
  inline T* GetPrev() { return static_cast<T*>(_Prev); }

protected:

   sChain() { _Prev=0; _Next=_Root; if (_Root) {_Root->_Prev=this;} _Root=this; } // link
  ~sChain() { if (_Next) {_Next->_Prev=_Prev;} if (_Prev) {_Prev->_Next=_Next;} else {_Root=_Next;} } // unlink

};

template <class T> sChain<T> * sChain<T>::_Root = 0;

#define sCHAIN_FOREACH(T,p) for (T *p = T::GetRoot(); p; p=p->GetNext())


*/

/****************************************************************************/
/***                                                                      ***/
/***   String Map                                                         ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   A simple hash table for mapping strings to pointers.               ***/
/***                                                                      ***/
/***   sStringMap_ maps to untyped pointers (void*), while the            ***/
/***   sStringMap  template allows storing of typed pointers              ***/
/***                                                                      ***/
/***   The number of buckets is fixed on intialization and                ***/
/***   should approximately equal the number of expected items.           ***/
/***   Initial memory consumption: numSlots * sizeof(void*)               ***/
/***   Speed and memory efficiency degrades gracefully (linear) with the  ***/
/***   number of inserted values.                                         ***/
/***                                                                      ***/
/***   Key strings will be copied on insertion (Set).                     ***/
/***   Setting a value for an already inserted key will overwrite the     ***/
/***   old value.                                                         ***/
/***                                                                      ***/
/****************************************************************************/

class sStringMap_
{
protected:

  sInt Size;
  typedef struct Slot_ {Slot_ * next; sChar * key; void * value;} Slot;
  Slot ** Slots;

  sU32 Hash (const sChar * key) const;

public: 

  sStringMap_ (sInt numSlots);    // creates a map with numSlots buckets
  virtual ~sStringMap_ ();
  void Clear ();                  // deletes all key value pairs

  void * Get (const sChar * key) const; // returns the value for key, or NULL if not set
  void   Set (const sChar * key, void * value); // sets the value for key
  void   Del (const sChar * key);    // removes the value for key

  sInt   GetCount () const;          // returns current number of key-value pairs
  sStaticArray<sChar*> * GetKeys () const; // returns a newly allocated array with 
                                     // all the keys in arbitrary order

  void Dump () const;
};

/****************************************************************************/

template <class Type, sInt NumSlots> class sStringMap : public sStringMap_
{
public:

  sStringMap () : sStringMap_(NumSlots) {};

  Type Get (const sChar * key) { return (Type)sStringMap_::Get(key); };
  void Set (const sChar * key, Type value) { sStringMap_::Set(key, (void *)value); };
};

/****************************************************************************/

/****************************************************************************/
/***                                                                      ***/
/*** sStream implementations                                              ***/
/***                                                                      ***/
/*** sStreamMemory   : reflects a block in memory                         ***/
/*** sStreamCache    : readahead/write cache for another stream           ***/
/*** sStreamPart     : reflects a part of another stream                  ***/
/*** sStreamTemp     : memory resident file                               ***/
/***                                                                      ***/
/****************************************************************************/
/*
class sStreamMemory : public sStream
{
protected:

  sU8 *Ptr;
  sDInt BSize, Pos;
  sBool Owner, Readonly;

public:

  sBool Open(sDInt size);   // creates new buffer
  sBool Open(const void *buffer, sDInt size, sBool iamowner=sFALSE); // opens buffer readonly
  sBool Open(void *buffer, sDInt size, sBool iamowner=sFALSE); // opens buffer read/write
  sBool Open(sStream &str); // loads other stream into memory

  sStreamMemory();
  explicit sStreamMemory(sDInt size);
  sStreamMemory(const void *buffer, sDInt size, sBool iamowner=sFALSE);
  sStreamMemory(void *buffer,sDInt size, sBool iamowner=sFALSE);
  explicit sStreamMemory(sStream &str);

  void  *Detach(sDInt *size);

  // sStream impl
  ~sStreamMemory();
  sInt  GetFlags();
  sBool  Close();
  sDInt Read(void *ptr, sDInt count);
  sDInt Write(const void *ptr, sDInt count);
  sSize Seek(sSize pos);
  sSize SeekEnd(sSize offset);
  sSize SeekCur(sSize offset);
  sBool EOF_();
  sSize Tell();
  sSize Size();
  const void *GetPtr(sDInt &count);
  sStream *Clone();

  using sStream::Read;
  using sStream::Write;
};


class sStreamPart : public sStream
{
protected:

  sStream *Str;
  sSize   PPos, PSize, Pos;
  sBool   Owner;

public:

  sBool Open(sStream &str, sSize from, sSize size, sBool clone=sFALSE);

  sStreamPart();
  sStreamPart(sStream &str, sSize from, sSize size, sBool clone=sFALSE);

  // sStream impl
  ~sStreamPart();
  sInt  GetFlags();
  sBool Close();
  sDInt Read(void *ptr, sDInt count);
  sDInt Write(const void *ptr, sDInt count);
  sSize Seek(sSize pos);
  sSize SeekEnd(sSize offset);
  sSize SeekCur(sSize offset);
  sBool EOF_();
  sSize Tell();
  sSize Size();
  sStream *Clone();

  using sStream::Read;
  using sStream::Write;
};


class sStreamCache : public sStream
{
protected:

  sStream *Str;
  sDInt BufSize;
  sU8  *WBuf, *RBuf;
  sDInt RBufPos, RBufSize;
  sDInt WBufSize;
  sInt Flags;

  void Flush();

public:

  sBool Open(sStream &str, sDInt granule=65536);

  sStreamCache();
  sStreamCache(sStream &str, sDInt granule=65536);

  // sStream impl
  ~sStreamCache();
  sInt  GetFlags();
  sBool Close();
  sDInt Read(void *ptr, sDInt count);
  sDInt Write(const void *ptr, sDInt count);
  sBool EOF_();

  using sStream::Read;
  using sStream::Write;
};


class sStreamTemp : public sStream
{
protected:

  static const sInt BUFSIZE=65536;

  struct Buffer
  {
    sU8 Mem[BUFSIZE];
    sDNode DNode;
  };

  sDList<Buffer,&Buffer::DNode> Nodes;
  
  sDInt FSize;
  Buffer *CurB;
  sDInt BPos,FPos;

public:

  sBool Open();
  sStreamTemp(sBool open=sFALSE);

  // sStream impl
  ~sStreamTemp();
  sInt  GetFlags();
  sBool Close();
  sDInt Read(void *ptr, sDInt count);
  sDInt Write(const void *ptr, sDInt count);
  sSize Seek(sSize pos);
  sSize Tell();
  sSize Size();
  sBool EOF_();
  sStream *Clone();

  using sStream::Read;
  using sStream::Write;
};

*/

/****************************************************************************/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/****************************************************************************/

#endif // FILE_BASE_TYPES2_HPP


