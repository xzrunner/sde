#ifndef _IS_SDE_TOOLS_STRING_TOOLS_H_
#define _IS_SDE_TOOLS_STRING_TOOLS_H_
#include "Tools.h"
namespace Tools
{
	class StringTools
	{
	public:
		static std::wstring stringToWString(const std::string& str);
		static std::string wstringToString(const std::wstring& wstr);

		// should less than 256
		static void writeString(const std::wstring& s, byte** data);
		static std::wstring readString(const byte* data, size_t& len);

		// erase ' ' and '\t' at endpoing of sin
		static std::string stringTrim(std::string& sIn);

		template<class T>
		static T substrFromBack(const T& src, const char start, const char end);

	};	// class StringTools

	template<class T>
	inline T StringTools::substrFromBack(const T& src, const char start, const char end)
	{
		T::size_type s = src.find_last_of(start),
				  e = src.find_last_of(end);
		return src.substr(++s, e - s);
	}
}
#endif	// _IS_SDE_TOOLS_STRING_TOOLS_H_