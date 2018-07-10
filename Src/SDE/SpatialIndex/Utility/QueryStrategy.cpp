#include "QueryStrategy.h"
#include "../../BasicType/Point3D.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex;

//
// class TraverseByLevelQueryStrategy
//

void TraverseByLevelQueryStrategy::getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
{
	const INode* n = dynamic_cast<const INode*>(&entry);
	if (n == NULL)
		throw Tools::IllegalStateException(
		"TraverseByLevelQueryStrategy::getNextEntry: Entry is not Node. "
		);

	//////////////////////////////////////////////////////////////////////////
	// do something
	//////////////////////////////////////////////////////////////////////////
	
	if (n->isIndex())
	{
		for (size_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
		{
			m_ids.push(n->getChildIdentifier(cChild));
		}
	}

	if (!m_ids.empty())
	{
		nextEntry = m_ids.front(); m_ids.pop();
		hasNext = true;
	}
	else
	{
		hasNext = false;
	}
}

//
// class LayerRegionQueryStrategy
//

void LayerRegionQueryStrategy::getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
{
	hasNext = false;

	IShape* ps;
	entry.getShape(&ps);
	ps->getMBR(m_indexedSpace);
	delete ps;
}

//
// class GetAllPoint3DQueryStrategy
//

void GetAllPoint3DQueryStrategy::getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
{
	const INode* n = dynamic_cast<const INode*>(&entry);
	if (n == NULL)
		throw Tools::IllegalStateException(
		"GetAllPoint3DQueryStrategy::getNextEntry: Entry is not Node. "
		);

	if (n->isIndex())
	{
		for (size_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
		{
			m_ids.push(n->getChildIdentifier(cChild));
		}
	}
	else
	{
		for (size_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
		{
			IShape* s;
			n->getChildShape(cChild, &s);

			Point3D* pPoint3D = dynamic_cast<Point3D*>(s);
			if (pPoint3D != NULL)
				m_points.push_back(pPoint3D);

			delete s;
		}
	}

	if (!m_ids.empty())
	{
		nextEntry = m_ids.front(); m_ids.pop();
		hasNext = true;
	}
	else
	{
		hasNext = false;
	}
}
