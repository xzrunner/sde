#ifndef _IS_SDE_DATA_SRC_SPATIAL_DB_H_
#define _IS_SDE_DATA_SRC_SPATIAL_DB_H_
#include "../Tools/SpatialDataBase.h"
#include "../BasicType/String.h"

namespace IS_SDE
{
	namespace DBFeature
	{
		class SpatialDB : public Tools::ISerializable
		{
		public:

			//
			// ISerializable interface
			//
			virtual size_t getByteArraySize() const;
			virtual void loadFromByteArray(const byte* data);
			virtual void storeToByteArray(byte** data, size_t& length) const;

			void open(const WString& filePath);

			//
			// for DataSrc
			//
			void createDataSrc(const WString& name);
			void deleteDataSrc(const WString& name);
			void getDataSrcList(std::vector<WString>* result) const;


		private:
			std::map<WString, size_t> m_dataSrcName;

		};	// class SpatialDB
	}
}

#endif // _IS_SDE_DATA_SRC_SPATIAL_DB_H_