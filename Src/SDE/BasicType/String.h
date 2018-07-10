#ifndef _IS_SDE_STRING_H_
#define _IS_SDE_STRING_H_
#include "../Tools/SpatialDataBase.h"
namespace IS_SDE
{
	class WString : public Tools::ISerializable
	{
	public:
		WString() {}
		WString(const std::wstring& wstr) : m_wstr(wstr) {}
		WString& operator = (const WString& ws) { m_wstr = ws.m_wstr; return *this; }

		//
		// ISerializable interface
		//
		virtual size_t getByteArraySize() const;
		virtual void loadFromByteArray(const byte* data);
		virtual void storeToByteArray(byte** data, size_t& length) const;

		void storeToByteArray(byte*& ptr) const;

		std::wstring wstr() { return m_wstr; }
		bool operator < (const WString& ws) const { return m_wstr < ws.m_wstr; }

	private:
		std::wstring m_wstr;

	};	// class WString
}
#endif	// _IS_SDE_STRING_H_