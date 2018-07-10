#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_DATA_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_DATA_H_
#include "../Base.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class Data : public IData, public Tools::ISerializable
			{
			public:
				Data(size_t len, byte* pData, Rect& r, id_type id, const ICondenseStrategy* strategy);
				Data(const IShape* s, id_type id, const ICondenseStrategy* strategy);
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

				const ICondenseStrategy* m_strategy;

			}; // Data
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_DATA_H_