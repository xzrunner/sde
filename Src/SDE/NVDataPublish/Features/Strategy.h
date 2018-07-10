#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_STRATEGY_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_STRATEGY_H_
#include "../Base.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class NoOffsetCondenseStrategy : public ICondenseStrategy
			{
			public:
				virtual size_t getType() const { return static_cast<size_t>(CS_NO_OFFSET); }
				virtual size_t allShapeDataSize(const INode* n) const;
				virtual size_t dataSize(const IShape* s) const;
				virtual void loadFromByteArray(IShape** s, const byte* data, size_t length) const;
				virtual void storeToByteArray(const IShape* s, byte** data, size_t& length) const;

			public:
				static const size_t TYPE_SIZE			= 1;
				static const size_t POINT_COUNT_SIZE	= 2;
				static const size_t RING_COUNT_SIZE		= 1;
				static const size_t COORDS_SIZE			= 4;

				static const size_t MAX_POINT_COUNT		= 0xffff;
				static const size_t MAX_RING_COUNT		= 0xff;

			}; // NoOffsetCondenseStrategy

//			class CertaintyOffsetCondenseStrategy : public ICondenseStrategy
//			{
//			public:
//				virtual size_t getType() const { return static_cast<size_t>(CS_CERTAINTY_OFFSET); }
//				virtual size_t allShapeDataSize(const INode* n) const;
//				virtual size_t dataSize(const IShape* s) const;
//				virtual void loadFromByteArray(IShape** s, const byte* data, size_t length) const;
//				virtual void storeToByteArray(const IShape* s, byte** data, size_t& length) const;
//			}; // CertaintyOffsetCondenseStrategy
//
//			class UncertaintyOffsetCondenseStrategy : public ICondenseStrategy
//			{
//			public:
//				virtual size_t getType() const { return static_cast<size_t>(CS_UNCERTAINTY_OFFSET); }
//				virtual size_t allShapeDataSize(const INode* n) const;
//				virtual size_t dataSize(const IShape* s) const;
//				virtual void loadFromByteArray(IShape** s, const byte* data, size_t length) const;
//				virtual void storeToByteArray(const IShape* s, byte** data, size_t& length) const;
//			}; // UncertaintyOffsetCondenseStrategy
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_STRATEGY_H_
