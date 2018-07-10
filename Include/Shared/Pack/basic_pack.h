// basic_pack.h
// pack the basic data-types
#ifndef IS_GIS_BASIC_PACK_H_
#define IS_GIS_BASIC_PACK_H_

#include "../../Common/basic_types.h"
#include "../SharedType/basic_gm_type.h"
#include "Packable.h"
//#include <string.h>


namespace IS_GIS{

class PackT_int_64 : public Packable
{
private:
	int_64 value;
public:
	operator int_64() const		{ return value;}
	void setValue(int_64 val)	{ value = val; }
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE) ;
};

class PackT_uint_64 : public Packable
{
private:
	uint_64 value;
public:
	operator uint_64() const	{  return value;}
	void setValue(uint_64 val)	{ value = val;}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_int_32 : public Packable
{
private:
	int_32 value;
public:
	operator int_32() const		{ return value;}
	void setValue(int_32 val)	{ value = val;}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};


class PackT_uint_32 : public Packable
{
private:
	uint_32 value;
public:
	operator uint_32() const		{ return value;}
	void setValue(uint_32 val)	{ value = val;}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_bool_32 : public Packable
{
private:
	bool_32 value;
public:
	operator bool_32() const	{ return value;}
	void setValue(bool_32 val)	{ value = val;}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_int_16 : public Packable
{
private:
	int_16 value;
public:
	operator int_16() const		{ return value;}
	void setValue(int_16 val)	{ value = val;}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_uint_16 : public Packable
{
private:
	uint_16 value;
public:
	operator uint_16() const		{ return value;}
	void setValue(uint_16 val)	{ value = val;}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_byte_8 : public Packable
{
private:
	byte_8 value;
public:
	operator byte_8() const		{ return value;}
	void setValue(byte_8 val)	{ value = val;}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_string : public Packable
{
private:
	std::string value;
public:
	operator std::string() const	{	return value;	}
	void setValue(const std::string &val)	{ value.resize(val.size()); value.assign(val);	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};


class PackT_wstring : public Packable
{
private:
	std::wstring value;
public:
	operator std::wstring() const {	return value;	}
	void setValue(const std::wstring &val)	{ value.resize(val.size()); value.assign(val);	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_double : public Packable
{
private:
	double value;
public:
	operator double() const				{	return value;	}
	void setValue(const double &val)	{	value = val;	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_float : public Packable
{
private:
	float value;
public:
	operator float() const				{	return value;	}
	void setValue(const float &val)		{	value = val;	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};
class PackT_GO_ID_ARRAY:public Packable
{
private:
	GO_ID_ARRAY value;
public:
	operator GO_ID_ARRAY() const		{	return value;	}
	void setValue(const GO_ID_ARRAY &val){	value = val;	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);

};
class PackT_Pos2D : public Packable
{
private:
	Pos2D value;
public:
	operator Pos2D() const				{	return value;	}
	void setValue(const Pos2D &val)		{	value = val;	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_Pos3D : public Packable
{
private:
	Pos3D value;
public:
	operator Pos3D() const				{	return value;	}
	void setValue(const Pos3D &val)		{	value = val;	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
};

class PackT_Rect2D : public Packable
{
private:
	Rect2D value;
public:
	operator Rect2D() const					{	return value;	}
	void setValue(const Rect2D &val)		{	value = val;	}
public:
	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);

};

typedef PackT_int_64 PackT_GO_ID;
typedef PackT_int_64 PackT_GM_ID;
typedef PackT_int_64 PackT_TS_Type;

} //namespace IS_GIS{

#endif //#ifndef IS_GIS_BASIC_PACK_H_