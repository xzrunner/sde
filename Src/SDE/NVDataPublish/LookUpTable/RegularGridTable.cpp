#include "RegularGridTable.h"
#include "TableBuilding.h"
#include "../../Render/Render.h"
#include "../../BasicType/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::LookUpTable;

RegularGridTable::RegularGridTable(IStorageManager& storage)
	: m_buffer(&storage)
{
	loadHeader();
}

RegularGridTable::RegularGridTable(IStorageManager& storage, size_t nodeSize, 
								   size_t capacity, const MapScope& scope)
	: m_buffer(&storage)
{
	initTableIndex(nodeSize, capacity, scope);
}

double RegularGridTable::queryMinDistance(const MapPos2D& sNodePos, size_t dGridID) const
{
	return TableBuilding::queryMinDistance(m_buffer, DATA_START_PAGE, m_gridNum, 
		getGridID(sNodePos), dGridID);
}

void RegularGridTable::loadTable(size_t destGridID, byte** data) const
{
	return TableBuilding::loadTable(m_buffer, DATA_START_PAGE, m_gridNum, destGridID, data);
}

double RegularGridTable::queryMinDistance(const MapPos2D& src, const byte* table) const
{
	return TableBuilding::queryMinDistance(getGridID(src), table);
}

// todo: maybe on the border
size_t RegularGridTable::getGridID(const MapPos2D& p) const
{
	assert(p.d0 >= m_mapScope.m_xMin && p.d1 >= m_mapScope.m_yMin);
	size_t	rx = static_cast<size_t>(floor( (p.d0 - m_mapScope.m_xMin) / m_edgeLength )),
			ry = static_cast<size_t>(floor( (p.d1 - m_mapScope.m_yMin) / m_edgeLength ));
	return rx + ry * m_horGridSize;
}

void RegularGridTable::outputIndexInfo(size_t& nextPage)
{
	storeHeader();
	nextPage = DATA_START_PAGE;
}

void RegularGridTable::drawTable(Render::GDIRender* render) const
{
	HDC tmpDC = render->getMemDC();
	SelectObject(tmpDC, CreatePen(PS_DASH, 1, RGB(255, 0, 0)));

	double x_s = m_mapScope.m_xMin, x_e = m_mapScope.m_xMax, 
		y_s = m_mapScope.m_yMin, y_e = m_mapScope.m_yMax;

	double x_curr = x_s;
	while (x_curr < x_e)
	{
		std::vector<MapPos2D> linePos;
		linePos.push_back(MapPos2D(x_curr, y_s));
		linePos.push_back(MapPos2D(x_curr, y_e));
		Line l(linePos);
		render->render(l);
		x_curr += m_edgeLength;
	}

	double y_curr = y_s;
	while (y_curr < y_e)
	{
		std::vector<MapPos2D> linePos;
		linePos.push_back(MapPos2D(x_s, y_curr));
		linePos.push_back(MapPos2D(x_e, y_curr));
		Line l(linePos);
		render->render(l);
		y_curr += m_edgeLength;
	}

	DeleteObject(SelectObject(tmpDC, GetStockObject(BLACK_PEN)));
	render->endMemRender();
}

void RegularGridTable::storeHeader()
{
	// consider this will less than pageSize
	const size_t headerSize =
		sizeof(size_t)		+		// m_gridNum
		sizeof(double) * 4	+		// m_mapScope
		sizeof(double)		+		// m_edgeLength
		sizeof(size_t);				// m_horGridSize

	byte* header = new byte[headerSize];
	byte* ptr = header;

	memcpy(ptr, &m_gridNum, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &m_mapScope.m_xMin, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_mapScope.m_yMin, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_mapScope.m_xMax, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_mapScope.m_yMax, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_edgeLength, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_horGridSize, sizeof(size_t));
	ptr += sizeof(size_t);

	id_type id = HEADER_PAGE;
	m_buffer->storeByteArray(id, headerSize, header);

	delete[] header;
}

void RegularGridTable::loadHeader()
{
	size_t headerSize;
	byte* header = 0;
	m_buffer->loadByteArray(HEADER_PAGE, headerSize, &header);

	byte* ptr = header;

	memcpy(&m_gridNum, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&m_mapScope.m_xMin, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_mapScope.m_yMin, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_mapScope.m_xMax, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_mapScope.m_yMax, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_edgeLength, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_horGridSize, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	delete[] header;
}

void RegularGridTable::initTableIndex(size_t cNode, size_t capacity, const MapScope& scope)
{
	m_mapScope = scope;
	size_t gridSize = static_cast<size_t>(std::ceil(static_cast<double>(cNode) / static_cast<double>(capacity)));
	m_edgeLength = std::sqrt(scope.xLength() * scope.yLength() / gridSize);
	m_horGridSize = static_cast<size_t>(std::ceil(scope.xLength() / m_edgeLength));
	m_gridNum = m_horGridSize * static_cast<size_t>(std::ceil(scope.yLength() / m_edgeLength));
}
