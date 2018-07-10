#ifndef _IS_SDE_SPATIALINDEX_DATA_H_
#define _IS_SDE_SPATIALINDEX_DATA_H_
#include "../../Tools/SpatialDataBase.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		class Data : public IData, public Tools::ISerializable
		{
		public:
			Data(size_t len, byte* pData, Rect& r, id_type id);
			Data(const IShape* s, id_type id);
			virtual ~Data();

			//
			// IObject interface
			//
			virtual Data* clone();

			//
			// IEntry interface
			//
			virtual id_type getIdentifier() const;
			virtual void getShape(IShape** out) const;

			//
			// IData interface
			//
			virtual void getData(size_t& len, byte** data) const;

			//
			// ISpatialIndex interface
			//
			virtual size_t getByteArraySize() const;
			virtual void loadFromByteArray(const byte* data);
			virtual void storeToByteArray(byte** data, size_t& len) const;

		private:
			void getCertainShape(IShape** s) const;

		public:
			id_type m_id;
			Rect m_rect;
			byte* m_pData;
			size_t m_dataLength;
		}; // Data
	}
}

#endif // _IS_SDE_SPATIALINDEX_DATA_H_

