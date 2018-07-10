#include "StringTools.h"
#include <windows.h>

using namespace Tools;

std::wstring StringTools::stringToWString(const std::string& str)
{
	wchar_t* wname;
	int len = MultiByteToWideChar(/*0 CP_UTF8*/CP_ACP, 0 , str.c_str() , -1 , NULL, 0);
	wname = new WCHAR[len];
	MultiByteToWideChar(/*0 CP_UTF8*/CP_ACP, 0 , str.c_str(), -1, wname , len);
	std::wstring temp_name(wname);

	delete []wname;
	return temp_name;
}

std::string StringTools::wstringToString(const std::wstring& wstr)
{
	int i = WideCharToMultiByte(/*0 CP_UTF8*/CP_ACP, 
		0, wstr.c_str(), -1, NULL, 0, NULL, NULL);		
	char *strD = new char[i]; 
	WideCharToMultiByte(/*0 CP_UTF8*/CP_ACP, 
		0, wstr.c_str(), -1, strD, i, NULL, NULL) ;			
	std::string str(strD );

	delete []strD;	
	return str;
}

void StringTools::writeString(const std::wstring& s, byte** data)
{
	size_t length = s.size();
	assert(length < 256);

	**data = (byte)length;
	*data += sizeof(byte);

	for (size_t i = 0; i < length; ++i)
	{
		memcpy(*data, &s[i], sizeof(wchar_t));
		*data += sizeof(wchar_t);
	}
}

std::wstring StringTools::readString(const byte* data, size_t& len)
{
	size_t length = *data;
	data += sizeof(byte);

	std::wstring ret;
	ret.resize(length);

	for (size_t i = 0; i < length; ++i)
	{
		memcpy(&ret[i], data, sizeof(wchar_t));
		data += sizeof(wchar_t);
	}

	len = sizeof(byte) + sizeof(wchar_t) * length;

	return ret;
}

std::string StringTools::stringTrim(std::string& sIn)
{
	int i = 0;
	size_t size;
	std::string str_out;

	size = sIn.size();
	if ( i == size ) 
		return std::string("");
	while(sIn[i] ==' '|| sIn[i] == '\t')
		i++;

	if (i == size) 
		return std::string("");
	else
	{
		str_out = sIn.substr(i);
		size = str_out.size();

		while (str_out[size-1] == ' '|| str_out[size-1] == '\t')
			size--;

		str_out = str_out.substr(0, size);
		return str_out;
	}
}