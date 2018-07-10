#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <assert.h>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <string>
#include <sstream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <list>
#include <algorithm>
#include <cmath>
#include <limits>
#include <climits>
#include <time.h>

typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#include "PointerPool.h"
#include "PoolPointer.h"
#include "SmartPointer.h"

#define NOMINMAX
#define interface class
typedef uint8_t byte;

namespace Tools
{
	const double DOUBLE_MAX			= std::numeric_limits<double>::max();
	const double DOUBLE_MIN			= std::numeric_limits<double>::min();

	const uint32_t UINT32_MAX		= std::numeric_limits<uint32_t>::max();

	const double SPATIAL_MATH_PI	= 3.14159265358979325;

	const double DOUBLE_TOLERANCE	= std::numeric_limits<double>::epsilon() * 100;
	const double PRJ_TOLERANCE		= 0.01;
	const double GEO_TOLERANCE		= 0.0000001;
	const double ANGLE_TOLEANCE		= SPATIAL_MATH_PI / 180 / 100;		// 0.01 degree

	// Current using
	const double DOUBLE_ZERO		= GEO_TOLERANCE;

	enum VariantType
	{
		VT_LONG = 0x0,
		VT_BYTE,
		VT_SHORT,
		VT_FLOAT,
		VT_DOUBLE,
		VT_CHAR,
		VT_USHORT,
		VT_ULONG,
		VT_INT,
		VT_UINT,
		VT_BOOL,
		VT_PCHAR,
		VT_PWCHAR,
		VT_PVOID,
		VT_EMPTY,
		VT_LONGLONG,
		VT_ULONGLONG
	};

	template<class T>
	struct DeletePointerFunctor
	{
		void operator()(const T* pT) const
		{
			delete pT, pT = NULL;
		}
	};

	//
	// Exceptions
	//
	class Exception
	{
	public:
		virtual std::string what() = 0;
		virtual ~Exception() {}
	}; // Exception

	class IndexOutOfBoundsException : public Exception
	{
	public:
		IndexOutOfBoundsException(size_t i);
		virtual ~IndexOutOfBoundsException() {}
		virtual std::string what();

	private:
		std::string m_error;
	}; // IndexOutOfBoundsException

	class IllegalArgumentException : public Exception
	{
	public:
		IllegalArgumentException(std::string s);
		virtual ~IllegalArgumentException() {}
		virtual std::string what();

	private:
		std::string m_error;
	}; // IllegalArgumentException

	class IllegalStateException : public Exception
	{
	public:
		IllegalStateException(std::string s);
		virtual ~IllegalStateException() {}
		virtual std::string what();

	private:
		std::string m_error;
	}; // IllegalStateException

	class EndOfStreamException : public Exception
	{
	public:
		EndOfStreamException(std::string s);
		virtual ~EndOfStreamException() {}
		virtual std::string what();

	private:
		std::string m_error;
	}; // EndOfStreamException

	class ResourceLockedException : public Exception
	{
	public:
		ResourceLockedException(std::string s);
		virtual ~ResourceLockedException() {}
		virtual std::string what();

	private:
		std::string m_error;
	}; // ResourceLockedException

	class InvalidPageException : public Exception
	{
	public:
		InvalidPageException(size_t id);
		virtual ~InvalidPageException() {}
		virtual std::string what();

	private:
		std::string m_error;
	}; // InvalidPageException

	class NotSupportedException : public Exception
	{
	public:
		NotSupportedException(std::string s);
		virtual ~NotSupportedException() {}
		virtual std::string what();

	private:
		std::string m_error;
	}; // NotSupportedException

	//
	// Interfaces
	//

	interface IObject
	{
	public:
		virtual ~IObject () {}
		virtual IObject* clone() = 0;
	}; // IObject

	interface ISerializable
	{
	public:
		virtual ~ISerializable() {}
		// returns the size of the required byte array.
		virtual size_t getByteArraySize() const = 0;
		// load this object using the byte array.
		virtual void loadFromByteArray(const byte* data) = 0;
		// store this object in the byte array.
		virtual void storeToByteArray(byte** data, size_t& length) const = 0;
	};	// ISerializable

	interface IComparable //: public virtual IObject
	{
	public:
		virtual ~IComparable() {}

		virtual bool operator<(const IComparable& o) const = 0;
		virtual bool operator>(const IComparable& o) const = 0;
		virtual bool operator==(const IComparable& o) const = 0;
	}; //IComparable

	interface IObjectComparator
	{
	public:
		virtual ~IObjectComparator() {}

		virtual int compare(IObject* o1, IObject* o2) = 0;
	}; // IObjectComparator

	interface IObjectStream
	{
	public:
		virtual ~IObjectStream() {}

		virtual IObject* getNext() = 0;
		// returns a pointer to the next entry in the
		// stream or 0 at the end of the stream.

		virtual bool hasNext() = 0;
		// returns true if there are more items in the stream.

		virtual size_t size() = 0;
		// returns the total number of entries available in the stream.

		virtual void rewind() = 0;
		// sets the stream pointer to the first entry, if possible.
	}; // IObjectStream

	//
	// Classes & Functions
	//

	class Variant
	{
	public:
		Variant();

		VariantType m_varType;

		union
		{
			int16_t iVal;              // VT_SHORT
			int32_t lVal;              // VT_LONG
			int64_t llVal;             // VT_LONGLONG
			byte bVal;                 // VT_BYTE
			float fltVal;              // VT_FLOAT
			double dblVal;             // VT_DOUBLE
			char cVal;                 // VT_CHAR
			uint16_t uiVal;            // VT_USHORT
			uint32_t ulVal;            // VT_ULONG
			uint64_t ullVal;           // VT_ULONGLONG
			bool blVal;                // VT_BOOL
			char* pcVal;               // VT_PCHAR
			wchar_t* pwcVal;           // VT_PWCHAR
			void* pvVal;               // VT_PVOID
		} m_val;
	}; // Variant

	class PropertySet;
	std::ostream& operator<<(std::ostream& os, const Tools::PropertySet& p);

	class PropertySet : public ISerializable
	{
	public:
		PropertySet() {}
		PropertySet(const byte* data);
		virtual ~PropertySet() {}

		//
		// ISerializable interface
		//
		virtual size_t getByteArraySize() const;
		virtual void loadFromByteArray(const byte* data);
		virtual void storeToByteArray(byte** data, size_t& length) const;

		Variant getProperty(const std::string& property);
		void setProperty(const std::string& property, Variant& v);
		void removeProperty(const std::string& property);

	private:
		std::map<std::string, Variant> m_propertySet;

		friend std::ostream& Tools::operator<<(
			std::ostream& os,
			const Tools::PropertySet& p
			);
	}; // PropertySet

	//class QueryVct
	//{
	//public:
	//private:
	//	std::vector<size_t, Variant>
	//}; // QueryVct

	class SharedLock
	{
	public:
#if HAVE_PTHREAD_H
		SharedLock(pthread_rwlock_t* pLock);
		~SharedLock();

	private:
		pthread_rwlock_t* m_pLock;
#endif
	}; // SharedLock

	class ExclusiveLock
	{
	public:
#if HAVE_PTHREAD_H
		ExclusiveLock(pthread_rwlock_t* pLock);
		~ExclusiveLock();

	private:
		pthread_rwlock_t* m_pLock;
#endif
	}; // ExclusiveLock

	IObjectStream* externalSort(IObjectStream& source, size_t bufferSize);
	IObjectStream* externalSort(IObjectStream& source, IObjectComparator& pComp, size_t bufferSize);
}

#endif // _TOOLS_H_