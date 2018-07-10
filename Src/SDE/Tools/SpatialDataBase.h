#ifndef _IS_SDE_TOOLS_SPATIAL_DATABASE_H_
#define _IS_SDE_TOOLS_SPATIAL_DATABASE_H_
#include "Tools.h"
#include "CompileConfig.h"
#include "../BasicType/BasicGMType.h"

# if !HAVE_BZERO
#  define bzero(d, n) memset((d), 0, (n))
# endif

namespace IS_SDE
{
	class Point;
	class Rect;

	typedef int64_t id_type;
	typedef Tools::PoolPointer<IS_SDE::Rect> RectPtr;
	typedef Tools::PoolPointer<IS_SDE::Point> PointPtr;

	enum CommandType
	{
		CT_NODEREAD = 0x0,
		CT_NODEDELETE,
		CT_NODEWRITE
	};

	enum ShapeType
	{
		ST_POINT = 0x0,
		ST_LINE,
		ST_POLYGON
	};

	enum SpatialIndexType
	{
		SIT_RTree = 0x0,
		SIT_QuadTree
	};

	//
	// Interfaces
	//

	interface ICrt
	{
	public:
		virtual void trans(const MapPos3D& src, MapPos3D* dest) const = 0;
		virtual ~ICrt() {}
	};

	interface IShape : public virtual Tools::ISerializable
	{
	public:
		virtual bool intersectsShape(const IShape& in) const = 0;
		virtual bool containsShape(const IShape& in) const = 0;
		virtual bool touchesShape(const IShape& in) const = 0;
		virtual void getCenter(Point& out) const = 0;
		virtual void getMBR(Rect& out) const = 0;
		virtual double getArea() const = 0;
		virtual double getMinimumDistance(const IShape& in) const = 0;
		virtual void transGeoToPrj(const ICrt& crt) = 0;
		virtual ~IShape() {}
	}; // IShape

//	interface IGeoShape : 

	interface IEntry : public Tools::IObject
	{
	public:
		virtual id_type getIdentifier() const = 0;
		virtual void getShape(IShape** out) const = 0;
		virtual ~IEntry() {}
	}; // IEntry

	interface INode : public IEntry, public Tools::ISerializable
	{
	public:
		virtual size_t getChildrenCount() const = 0;
		virtual id_type getChildIdentifier(size_t index) const = 0;
		virtual void setChildIdentifier(size_t index, id_type id) = 0;
		virtual void getChildShape(id_type id, IShape** out) const = 0;
		virtual void getChildShape(size_t index, IShape** out) const = 0;
		virtual size_t getLevel() const = 0;
		virtual bool isIndex() const = 0;
		virtual bool isLeaf() const = 0;
		virtual ~INode() {}
	}; // INode

	interface IData : public IEntry
	{
	public:
		virtual void getData(size_t& len, byte** data) const = 0;
		virtual ~IData() {}
	}; // IData

	interface IDataStream : public Tools::IObjectStream
	{
	public:
		virtual IData* getNext() = 0;
		virtual ~IDataStream() {}
	}; // IDataStream

	interface ICommand
	{
	public:
		virtual void execute(const INode& in) = 0;
		virtual ~ICommand() {}
	}; // ICommand

	interface INearestNeighborComparator
	{
	public:
		virtual double getMinimumDistance(const IShape& query, const IShape& entry) = 0;
		virtual double getMinimumDistance(const IShape& query, const IData& data) = 0;
		virtual ~INearestNeighborComparator() {}
	}; // INearestNeighborComparator

	interface IStorageManager
	{
	public:
		virtual void loadByteArray(const id_type id, size_t& len, byte** data) = 0;
		virtual void storeByteArray(id_type& id, const size_t len, const byte* const data) = 0;
		virtual void deleteByteArray(const id_type id) = 0;
		//virtual void getNodePageInfo(id_type id, size_t& length, size_t& pageSize) const = 0;
		virtual ~IStorageManager() {}
	}; // IStorageManager

	interface IVisitor
	{
	public:
		virtual bool shouldCheckEachData() const = 0;
		virtual void visitNode(const INode& in) = 0;
		//virtual void visitData(const IData& in) = 0;
		virtual void visitData(const IData& in, size_t dataIndex) = 0;
		virtual void visitData(std::vector<const IData*>& v) = 0;
		virtual void visitData(const INode& in) = 0;
		virtual ~IVisitor() {}
	}; // IVisitor

	interface ICountVisitor
	{
	public:
		virtual void countNode(const INode& in) = 0;
		virtual void countData(const INode& in, size_t iData, const IShape& query) = 0;
		virtual bool overUpperLimit() const = 0;
		virtual ~ICountVisitor() {}
	}; // ICountVisitor

	interface IQueryStrategy
	{
	public:
		virtual void getNextEntry(const IEntry& previouslyFetched, id_type& nextEntryToFetch, bool& bFetchNextEntry) = 0;
		virtual bool shouldStoreEntry() = 0;
		virtual ~IQueryStrategy() {}
	}; // IQueryStrategy

	interface IQueryStoreStrategy
	{
	public:
//		virtual void getNextEntry(const IStorageManager* sm, const IEntry& previouslyFetched, id_type& nextEntryToFetch, bool& bFetchNextEntry) = 0;
//		virtual void query
		virtual ~IQueryStoreStrategy() {}
	}; // IQueryStoreStrategy

	//interface IRender
	//{
	//public:
	//	virtual void render(const IShape& s) const = 0;
	//	virtual ~IRender() {}
	//}; // IRender

	//interface IRenderStyle
	//{
	//public:
	//	virtual void selectStyle(IRender& render) const = 0;
	//	virtual void deleteStyle(IRender& render) const = 0;
	//	virtual ~IRenderStyle() {}
	//}; // IRenderStyle

	interface IStatistics
	{
	public:
		virtual size_t getReads() const = 0;
		virtual size_t getWrites() const = 0;
		virtual size_t getNumberOfNodes() const = 0;
		virtual size_t getNumberOfData() const = 0;
		virtual ~IStatistics() {}
	}; // IStatistics

	interface ISpatialIndex
	{
	public:
		virtual void insertData(size_t len, const byte* pData, const IShape& shape, id_type objID, id_type* nodeID) = 0;
		virtual bool deleteData(const IShape& shape, id_type shapeIdentifier) = 0;
		virtual void containsWhatQuery(const IShape& query, IVisitor& v, bool isFine)  = 0;
		virtual void intersectsWithQuery(const IShape& query, IVisitor& v, bool isFine) = 0;
		virtual void pointLocationQuery(const Point& query, IVisitor& v, bool isFine) = 0;
		virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator& nnc) = 0;
		virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v) = 0;
		virtual void selfJoinQuery(const IShape& s, IVisitor& v) = 0;
		virtual void queryStrategy(IQueryStrategy& qs) = 0;
		virtual bool countIntersectsQuery(const IShape& query, ICountVisitor& v) = 0;
		virtual void getIndexProperties(Tools::PropertySet& out) const = 0;
		virtual void addCommand(ICommand* in, CommandType ct) = 0;
		virtual bool isIndexValid() = 0;
		virtual void getStatistics(IStatistics** out) const = 0;

//		virtual void checkStoreState(ICheckStoreStrategy& css) = 0;
		virtual void readData(id_type nodeID, id_type objID, IShape** out) = 0;
		virtual id_type nextObjID() = 0;
		virtual void setDisplayRegion(const Rect& scope) = 0;
		virtual const Rect& getDisplayRegion() = 0;

		virtual ~ISpatialIndex() {}

	}; // ISpatialIndex

	interface IFields
	{
	public:
		virtual size_t getIndex(const std::string& fieldName) const = 0;
		virtual ~IFields() {}
	}; // IFields

	// A field is part of row, many fields compose a row.
	interface IField
	{
	public:
		virtual Tools::VariantType getType() const = 0;
		virtual void setVal(const Tools::Variant& val) = 0;
		virtual void getVal(IField& val) const = 0;
		virtual ~IField() {}
	}; // IField

	// A row is a record in a table. All rows in a table
	// share the same set of fields.
	interface IRow : public Tools::ISerializable
	{
	public:
		virtual size_t getFieldNum() const = 0;
		virtual void getField(size_t index, IField& val) const = 0;
		virtual void getField(const std::string& field, IField& val) const = 0;
		virtual ~IRow() {}
	}; // IRow

	template<class T>
	interface INonSpatialIndex
	{
	public:
		virtual void insertData(const T& key, size_t len, const byte* pData) = 0;
		virtual bool deleteData(const T& key) = 0;
		virtual bool equalsQuery(const T& key, IRow& r) = 0;
		virtual ~INonSpatialIndex() {}
	}; // INonSpatialIndex

	namespace StorageManager
	{
		enum StorageManagerConstants
		{
			EmptyPage = -0x1,
			NewPage = -0x1
		};

		interface IBuffer : public IStorageManager
		{
		public:
			virtual size_t getHits() = 0;
			virtual void clear() = 0;
			virtual ~IBuffer() {}
		}; // IBuffer

		extern IStorageManager* returnMemoryStorageManager(Tools::PropertySet& in);
		extern IStorageManager* createNewMemoryStorageManager();

		extern IStorageManager* returnDiskStorageManager(Tools::PropertySet& in);
		extern IStorageManager* createNewDiskStorageManager(const std::wstring& baseName, size_t pageSize);
		extern IStorageManager* loadDiskStorageManager(const std::wstring& baseName);

		extern IStorageManager* returnStaticDiskStorageManager(Tools::PropertySet& in);
		extern IStorageManager* createNewStaticDiskStorageManager(const std::wstring& baseName, size_t pageSize);
		extern IStorageManager* loadStaticDiskStorageManager(const std::wstring& baseName);

		extern IBuffer* returnRandomEvictionsBuffer(IStorageManager& ind, Tools::PropertySet& in);
		extern IBuffer* createNewRandomEvictionsBuffer(IStorageManager& in, size_t capacity, bool bWriteThrough);

		extern IBuffer* returnFIFOBuffer(IStorageManager& ind, Tools::PropertySet& in);
		extern IBuffer* createNewFIFOBuffer(IStorageManager& in, size_t capacity, bool bWriteThrough);
	}
}

#endif // _IS_SDE_TOOLS_SPATIAL_DATABASE_H_

