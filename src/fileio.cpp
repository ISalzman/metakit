// fileio.cpp --
// $Id: fileio.cpp 1268 2007-03-09 16:53:24Z jcw $
// This is part of MetaKit, see http://www.equi4.com/metakit/

/** @file
 * Implementation of c4_FileStrategy and c4_FileStream
 */

#include "header.h"
#include "mk4io.h"

#if q4_WIN32
#if q4_MSVC && !q4_STRICT
#pragma warning(disable: 4201) // nonstandard extension used : ...
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#if q4_UNIX && HAVE_MMAP
#include <sys/types.h>
#include <sys/mman.h>
#endif

#if q4_UNIX
#include <unistd.h>
#include <fcntl.h>
#endif

#include <time.h>

/////////////////////////////////////////////////////////////////////////////

#if q4_CHECK
#include <stdlib.h>

void f4_AssertionFailed(const char* cond_, const char* file_, int line_)
{
  fprintf(stderr, "Assertion failed: %s (file %s, line %d)\n",
            cond_, file_, line_);
  abort();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// c4_FileStream

c4_FileStream::c4_FileStream (FILE* stream_, bool owned_)
  : _stream (stream_), _owned (owned_)
{
}

c4_FileStream::~c4_FileStream ()
{
  if (_owned)
    fclose(_stream);
}

int c4_FileStream::Read(void* buffer_, int length_)
{
  d4_assert(_stream != 0);

  return (int) fread(buffer_, 1, length_, _stream);
}

bool c4_FileStream::Write(const void* buffer_, int length_)
{
  d4_assert(_stream != 0);

  return (int) fwrite(buffer_, 1, length_, _stream) == length_;
}

/////////////////////////////////////////////////////////////////////////////
// c4_FileStrategy

c4_FileStrategy::c4_FileStrategy (FILE* file_)
  : _file (file_), _cleanup (0)
{
  ResetFileMapping();
}

c4_FileStrategy::~c4_FileStrategy ()
{
  _file = 0;
  ResetFileMapping();

  if (_cleanup)
    fclose(_cleanup);

  d4_assert(_mapStart == 0);
}

bool c4_FileStrategy::IsValid() const
{ 
  return _file != 0; 
}

t4_i32 c4_FileStrategy::FileSize()
{
  d4_assert(_file != 0);

  long size = -1;

  long old = ftell(_file);
  if (old >= 0 && fseek(_file, 0, 2) == 0) {
    long pos = ftell(_file);
    if (fseek(_file, old, 0) == 0)
      size = pos;
  }

  if (size < 0)
  _failure = ferror(_file);

  return size;
}

t4_i32 c4_FileStrategy::FreshGeneration()
{
  return time(0);
}

void c4_FileStrategy::ResetFileMapping()
{
#if q4_WIN32
  if (_mapStart != 0) {
    _mapStart -= _baseOffset;
    d4_dbgdef(BOOL g =)
      ::UnmapViewOfFile((char*) _mapStart);
    d4_assert(g);
    _mapStart = 0;
    _dataSize = 0;
  }

  if (_file != 0) {
    t4_i32 len = FileSize();

    if (len > 0) {
      FlushFileBuffers((HANDLE) _get_osfhandle(_fileno(_file)));
      HANDLE h = ::CreateFileMapping((HANDLE) _get_osfhandle(_fileno(_file)),
                        0, PAGE_READONLY, 0, len, 0);
      d4_assert(h); // check for errors, but can continue without mapping

      if (h) {
        _mapStart = (t4_byte*) ::MapViewOfFile(h, FILE_MAP_READ, 0, 0, len);
        d4_assert(_mapStart != 0);

        if (_mapStart != 0) {
	  _mapStart += _baseOffset;
	  _dataSize = len - _baseOffset;
	}

        d4_dbgdef(BOOL f =)
	  ::CloseHandle(h);
        d4_assert(f);
      }
    }
  }
#elif q4_UNIX && HAVE_MMAP
  if (_mapStart != 0) {
    _mapStart -= _baseOffset;
    munmap((char*) _mapStart, _baseOffset + _dataSize); // also loses const
    _mapStart = 0;
    _dataSize = 0;
  }

  if (_file != 0) {
    t4_i32 len = FileSize();

    if (len > 0) {
      _mapStart = (const t4_byte*) mmap(0, len, PROT_READ, MAP_SHARED,
                            fileno(_file), 0);
      if (_mapStart != (void*) -1L) {
	_mapStart += _baseOffset;
	_dataSize = len - _baseOffset;
      } else
	_mapStart = 0;
    }
  }
#endif
}

bool c4_FileStrategy::DataOpen(const char* fname_, int mode_)
{
  d4_assert(!_file);

#if q4_WIN32 && !q4_BORC
  int flags = _O_BINARY | _O_NOINHERIT | (mode_ > 0 ? _O_RDWR : _O_RDONLY);
  int fd = _open(fname_, flags);
  if (fd != -1)
    _cleanup = _file = _fdopen(fd, mode_ > 0 ? "r+b" : "rb");
#else
  _cleanup = _file = fopen(fname_, mode_ > 0 ? "r+b" : "rb");
#if q4_UNIX
  if (_file != 0)
    fcntl(fileno(_file), F_SETFD, FD_CLOEXEC);
#endif
#endif

  if (_file != 0) {
    setbuf(_file, 0); // 30-11-2001
    ResetFileMapping();
    return true;
  }

  if (mode_ > 0) {
#if q4_WIN32 && !q4_BORC
    fd = _open(fname_, flags | _O_CREAT, _S_IREAD | _S_IWRITE);
    if (fd != -1)
      _cleanup = _file = _fdopen(fd, "w+b");
#else
    _cleanup = _file = fopen(fname_, "w+b");
#if q4_UNIX
    if (_file != 0)
      fcntl(fileno(_file), F_SETFD, FD_CLOEXEC);
#endif
#endif
  }

  if (_file != 0)
    setbuf(_file, 0); // 30-11-2001

  //d4_assert(_file != 0);
  return false;
}

int c4_FileStrategy::DataRead(t4_i32 pos_, void* buf_, int len_)
{
  d4_assert(_baseOffset + pos_ >= 0);
  d4_assert(_file != 0);

 //printf("DataRead at %d len %d\n", pos_, len_);
  return fseek(_file, _baseOffset + pos_, 0) != 0 ? -1 :
    (int) fread(buf_, 1, len_, _file);
}

void c4_FileStrategy::DataWrite(t4_i32 pos_, const void* buf_, int len_)
{
  d4_assert(_baseOffset + pos_ >= 0);
  d4_assert(_file != 0);
 //printf("DataWrite at %d len %d\n", pos_, len_);

#if q4_WIN32 || __MACH__
// if (buf_ >= _mapStart && buf_ <= _mapLimit - len_)

    // a horrendous hack to allow file mapping for Win95 on network drive
    // must use a temp buf to avoid write from mapped file to same file
    // 
    //  6-Feb-1999  --  this workaround is not thread safe
    // 30-Nov-2001  --  changed to use the stack so now it is
  char tempBuf [4096];
  d4_assert(len_ <= sizeof tempBuf);
  buf_ = memcpy(tempBuf, buf_, len_);
#endif

  if (fseek(_file, _baseOffset + pos_, 0) != 0 ||
      (int) fwrite(buf_, 1, len_, _file) != len_) {
    _failure = ferror(_file);
    d4_assert(_failure != 0);
    d4_assert(true); // always force an assertion failure in debug mode
  }
}

void c4_FileStrategy::DataCommit(t4_i32 limit_)
{
  d4_assert(_file != 0);

  if (fflush(_file) < 0) {
    _failure = ferror(_file);
    d4_assert(_failure != 0);
    d4_assert(true); // always force an assertion failure in debug mode
    return;
  }

  if (limit_ > 0) {
#if 0 // can't truncate file in a portable way!
      // unmap the file first, WinNT is more picky about this than Win95
    FILE* save = _file;

    _file = 0;
    ResetFileMapping();
    _file = save;

    _file->SetLength(limit_); // now we can resize the file
#endif
    ResetFileMapping(); // remap, since file length may have changed
  }
}

/////////////////////////////////////////////////////////////////////////////
