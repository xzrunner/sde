#ifndef _IS_SDE_SPATIALINDEX_QUADTREE_BULK_LOADER_H_
#define _IS_SDE_SPATIALINDEX_QUADTREE_BULK_LOADER_H_

#include "../../Tools/SmartPointer.h"
#include "../../Tools/Tools.h"
#include "../../Algorithms/IO/TemporaryFile.h"
#include "../Utility/BulkLoader.h"

using namespace Tools;

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class StoreObjs
			{
			public:
				StoreObjs();
				~StoreObjs();
				void storeNextObject(IObject* obj);

			private:
				Algorithms::TemporaryFile* m_shouldRefetchedObjs;
				size_t m_cNumOfShouldRefetchedObjs;
				IObject* m_pTemplateObj;

				friend class RefetchObjs;
			}; // StoreObj

			class RefetchObjs : public IObjectStream
			{
			public:
				RefetchObjs();
				RefetchObjs(RefetchObjs& refetchObjs);
				virtual ~RefetchObjs();
				RefetchObjs& operator = (StoreObjs& storeObjs);

				virtual IObject* getNext();
				virtual bool hasNext() throw ();
				virtual size_t size() throw (NotSupportedException);
				virtual void rewind();

				bool readComplete() const;
				void clear();

			private:
				Algorithms::TemporaryFile* m_objs;
				IObject* m_pTemplateObj;
				size_t m_cNumOfRead;
				size_t m_cTotNum;
			}; // RefetchObjs

			class BulkLoadSource : public IObjectStream
			{
			public:
				// if  isX region is at x (-max(double), endCoor)
				// if !isX region is at y (-max(double), endCoor)
				BulkLoadSource(SmartPointer<IObjectStream> spStream, double endCoor, bool isScaleX, RefetchObjs& lastObjs);
				virtual ~BulkLoadSource();

				virtual IObject* getNext();
				virtual bool hasNext() throw ();
				virtual size_t size() throw (NotSupportedException);
				virtual void rewind() throw (NotSupportedException);

				void readFinish();
				void loadDataToBufferedObj();

			private:
				SmartPointer<IObjectStream> m_spDataSource;
				IObject* m_pTemplateObj;
				double m_endCoor;
				bool m_scaleIsX;

				StoreObjs m_refetchObjs;
				RefetchObjs m_lastObjs;

				friend class BulkLoader;
			};	// BulkLoadSource

			//class BulkLoadSourceDisk
			//{

			//}; // BulkLoadSourceDisk

			class BulkLoadSourceMem
			{
			public:
				BulkLoadSourceMem(SmartPointer<IObjectStream> spStream, bool isScaleX);

				Tools::SmartPointer<IData> getNext();
				bool isFinished() const;

				void loadDataToTemplateObj();
				void setEndcoor(double endCoor);

			private:
				SmartPointer<IObjectStream> m_spDataSource;
				Tools::SmartPointer<IData> m_pTemplateObj;

				std::list<Tools::SmartPointer<IData> > m_refetcher;
				size_t m_remainNumOfRefetchedObj;
				std::list<Tools::SmartPointer<IData> >::iterator m_itrRefetcher;
				bool m_isTemplateObjFromRefetcher;

				double m_endCoor;
				bool m_scaleIsX;
			}; // BulkLoadSourceMem

			class BulkLoader
			{
			public:
				// [The Strategy of Decreasing Times of Reconstruct]
				// If object inserted outside of the root's MBR, the tree will be reconstruct.
				// Not consider overflow node, it also require reconstruct tree after overflow 
				// node has no room. So enlarge the root's MBR, the edge is twice as length as 
				// normal structure. But it will waste time at querying process, because each 
				// node will deeper one layer than normal structure. 
				void bulkLoadUsingType0(
					QuadTree* pTree,
					IDataStream& stream,
					size_t leafCapacity,
					size_t bufferSize
					);

			private:
				void buildEmptyQuadTree(					
					QuadTree* pTree,
					size_t leafCapacity,
					size_t bufferSize,
					size_t cLevel,
					const Rect& rootMBR
					) const;

				void getTotReigon(
					QuadTree* pTree,
					IDataStream& stream, 
					size_t leafCapacity,
					Rect* r,
					size_t& cLevel,
					size_t& cNodes,
					size_t& cTotData
					) const;


				void computeRegion(const Rect& src, size_t id, Rect* dest) const;

				size_t computeNodeID(size_t xIndex, size_t yIndex, size_t cLeavesEachDim) const;

				void createLeafLevel(
					QuadTree* pTree,
					IObjectStream& stream,
					size_t leafCapacity,
					size_t bufferSize,
					size_t cLeavesEachDim,
					size_t xIndex,
					const Rect& rootMBR,
					bool compX,
					bool needMoreLevel
					) const;

				void createNode(
					QuadTree* pTree,
					id_type id,
					std::vector<Tools::SmartPointer<IData> >& e,
					size_t leafCapacity
					) const;

				void condenseTree(QuadTree* pTree, size_t cLevel, size_t leafCapacity) const;

				friend class BulkLoadSource;
			};
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUADTREE_BULK_LOADER_H_
