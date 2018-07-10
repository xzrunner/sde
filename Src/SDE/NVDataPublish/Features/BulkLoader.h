#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_BULK_LOADER_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_BULK_LOADER_H_
#include "../Base.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace UtiBL
		{
			class TmpFile;
		}
	}

	namespace SimulateNavigate
	{
		namespace SpatialIndex
		{
			namespace StaticRTree
			{
				class StaticRTree;
				class Node;
			}
		}
	}

	namespace NVDataPublish
	{
		namespace Features
		{
			class BulkLoadSource : public Tools::IObjectStream
			{
			public:
				BulkLoadSource(Tools::SmartPointer<IObjectStream> spStream, size_t howMany);
				BulkLoadSource(IObjectStream* pStream, size_t howMany);
				BulkLoadSource(IObjectStream* pStream);
				virtual ~BulkLoadSource();

				virtual Tools::IObject* getNext();
				virtual bool hasNext() throw ();
				virtual size_t size() throw (Tools::NotSupportedException);
				virtual void rewind() throw (Tools::NotSupportedException);

				Tools::SmartPointer<IObjectStream> m_spDataSource;
				size_t m_cHowMany;
			};	// BulkLoadSource

			class CondenseData;
			class LayerBulkLoader
			{
			public:
				void bulkLoadUsingSTR(
					STATIC_LAYER* pTree,
					IDataStream& stream,
					CondenseData& condenseData,
					size_t bufferSize
					);

			public:
				static const size_t CHILDREN_COUNT_SIZE		= 2;
				static const size_t MBR_SIZE				= 2 * 2 * 4;
				static const size_t EACH_CHILD_SIZE_SIZE	= 3;

			protected:
				void partitionFace(
					STATIC_LAYER* pTree,
					Tools::IObjectStream& es,
					CondenseData& condenseData,
					size_t bufferSize,
					size_t level,
					SpatialIndex::UtiBL::TmpFile& tmpFile,
					size_t& numberOfNodes
					);

				void partitionLine(
					STATIC_LAYER* pTree,
					Tools::IObjectStream& es,
					CondenseData& condenseData,
					size_t bufferSize,
					size_t level,
					SpatialIndex::UtiBL::TmpFile& tmpFile,
					size_t& numberOfNodes
					);

				void createNode(
					SimulateNavigate::SpatialIndex::StaticRTree::Node* n,
					STATIC_LAYER* pTree,
					std::vector<std::pair<Tools::SmartPointer<IData>, size_t> >& e,
					size_t layerID,
					const NVDataPublish::ICondenseStrategy& strategy,
					size_t level,
					const ICrt* trans
					);

			}; // LayerBulkLoader
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_BULK_LOADER_H_
