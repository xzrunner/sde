#include "Line.h"
#include "Point.h"
#include "Math.h"
#include "../../SpatialIndex/Utility/Visitor.h"
#include "../../Tools/SpatialMath.h"
#include "../../BasicType/Line.h"

using namespace IS_SDE::NVDataPublish::Arrangement;

void Edge::init(Node* pBNode, Node* pENode, PolyEdge* pPolyEdge, const double& precision)
{
	m_begin = pBNode;
	m_end = pENode;
	m_prev = NULL;
	m_next = NULL;
	m_pPolyEdge = pPolyEdge;
	addEdgeToNode();
	m_scanInfo.init(pBNode->pos(), pENode->pos(), precision);
}

void Edge::clear()
{
}

const MapPos2D& Edge::beginPos() const
{
	return m_begin->pos();
}

const MapPos2D& Edge::endPos() const
{
	return m_end->pos();
}

Edge* Edge::cut(Node* n, const double& precision)
{
	assert(!Tools::SpatialMath::isTheSamePos(n->pos(), beginPos(), precision)
		&& !Tools::SpatialMath::isTheSamePos(n->pos(), endPos(), precision));

	if (isBeginHigher())
	{
		Edge* nextHalf = new Edge;

		nextHalf->m_begin = n;
		nextHalf->m_end = m_end;
		m_end->eraseEdge(this, false);
		m_end = n;
		n->addEdge(this, false);
		nextHalf->addEdgeToNode();

		if (m_next)
		{
			m_next->m_prev = nextHalf;
			nextHalf->m_next = m_next;
		}
		else
		{
			nextHalf->m_next = NULL;
		}
		m_next = nextHalf;
		nextHalf->m_prev = this;

		nextHalf->m_pPolyEdge = m_pPolyEdge;

		m_scanInfo.init(beginPos(), endPos(), precision);
		nextHalf->m_scanInfo.init(nextHalf->beginPos(), nextHalf->endPos(), precision);

		return nextHalf;
	}
	else
	{
		Edge* prevHalf = new Edge;

		prevHalf->m_begin = m_begin;
		prevHalf->m_end = n;
		m_begin->eraseEdge(this, true);
		m_begin = n;
		n->addEdge(this, true);
		prevHalf->addEdgeToNode();

		if (m_prev)
		{
			m_prev->m_next = prevHalf;
			prevHalf->m_prev = m_prev;
		}
		else
		{
			prevHalf->m_prev = NULL;
		}
		m_prev = prevHalf;
		prevHalf->m_next = this;

		prevHalf->m_pPolyEdge = m_pPolyEdge;

		m_scanInfo.init(beginPos(), endPos(), precision);
		prevHalf->m_scanInfo.init(prevHalf->beginPos(), prevHalf->endPos(), precision);

		if (m_pPolyEdge->m_begin == this)
			m_pPolyEdge->m_begin = prevHalf;

		return prevHalf;
	}
}

void Edge::setDirtyState()
{
	m_pPolyEdge->m_bDirty = true;
}

bool Edge::shouldEraseDuplicated() const
{
	std::set<Edge*>::iterator itr = m_end->m_begin.begin();
	for ( ; itr != m_end->m_begin.end(); ++itr)
	{
		Edge* e = *itr;
		if (m_begin == e->m_end && isLowerGradeThan(*e))
			return true;
	}

	itr = m_end->m_end.begin();
	for ( ; itr != m_end->m_end.end(); ++itr)
	{
		if (*itr == this)
			continue;

		Edge* e = *itr;
		if (m_begin == e->m_begin && isLowerGradeThan(*e))
			return true;
	}

	return false;
}

bool Edge::isSameAttr(const Edge* e) const
{
	return m_pPolyEdge->m_pGeoLine->name() == e->m_pPolyEdge->m_pGeoLine->name();
}

void Edge::addEdgeToNode()
{
	m_begin->addEdge(this, true);
	m_end->addEdge(this, false);
}

bool Edge::isLowerGradeThan(const Edge& e) const
{
	if (m_pPolyEdge->m_pGeoLine->m_layerID > e.m_pPolyEdge->m_pGeoLine->m_layerID)
		return true;
	//if (m_pPolyEdge->m_pGeoLine->m_layerID > e.m_pPolyEdge->m_pGeoLine->m_layerID)
	//	return false;

	if (m_pPolyEdge->m_pGeoLine != e.m_pPolyEdge->m_pGeoLine)
	{
		return m_pPolyEdge->m_pGeoLine < e.m_pPolyEdge->m_pGeoLine;
	}
	else
	{
		assert(this != &e);

		Edge* tmp = m_next;
		while (tmp)
		{
			if (tmp == &e)
				return true;
			tmp = tmp->m_next;
		}

		return false;
	}
}

void Edge::ScanInfo::init(const MapPos2D& begin, const MapPos2D& end, const double& precision)
{
	//double	dx = begin.d0 - end.d0, 
	//		dy = begin.d1 - end.d1;
	//if (fabs(dy) > Tools::DOUBLE_TOLERANCE)
	//	slope = - dx / dy;
	//else
	//	slope = Tools::DOUBLE_MAX;

	//if (slope == Tools::DOUBLE_MAX)
	//	bStartHigher = begin.d0 < end.d0;
	//else
	//	bStartHigher = Math::isPosHigher(begin, end);

	double	dx = begin.d0 - end.d0, 
			dy = begin.d1 - end.d1;
	if (fabs(dy) > precision + precision)
		slope = - dx / dy;
	else
		slope = Tools::DOUBLE_MAX;

	bStartHigher = Math::isPosHigher(begin, end);
}

void Edge::ScanInfo::update(const MapPos2D& begin, const MapPos2D& end)
{
	if (slope == Tools::DOUBLE_MAX)
	{
		bStartHigher = Math::isPosHigher(begin, end);
	}
	else
	{
		double	dx = begin.d0 - end.d0, 
				dy = begin.d1 - end.d1;
		slope = - dx / dy;
		assert(bStartHigher == Math::isPosHigher(begin, end));
	}
}

//////////////////////////////////////////////////////////////////////////

bool PolyEdge::init(GeoLine* pGeoLine, NodeMgr* pNodeMgr, std::stack<Edge*>* edgePool, const double& precision)
{
	m_pGeoLine	= pGeoLine;
	m_bDirty	= false;
	m_type		= COMMON;
	m_countEdge = 0;


	std::vector<MapPos2D> ctlPos;
	pGeoLine->getCoordinates(&ctlPos);

	Node* lastNode;
	pNodeMgr->createNode(ctlPos.front(), &lastNode);
	
	if (lastNode->pos() != ctlPos.front())
		m_bDirty = true;
	size_t currIndex = 1;
	bool bStart = false;

	Edge *lastEdge = NULL;
	
	while (true)
	{
		Node* currNode;
		pNodeMgr->createNode(ctlPos.at(currIndex), &currNode);

		if (currNode->pos() != ctlPos.at(currIndex))
			m_bDirty = true;

		if (currNode == lastNode)
		{
			++currIndex;
			if (currIndex == ctlPos.size())
				break;
			continue;
		}

		Edge* currEdge;
		if (!edgePool->empty())
		{
			currEdge = edgePool->top(); edgePool->pop();
		}
		else
		{
			currEdge = new Edge();
		}
		currEdge->init(lastNode, currNode, this, precision);

		if (!bStart)
		{
			bStart = true;
			m_begin = currEdge;
		}
		else
		{
			lastEdge->m_next = currEdge;
			currEdge->m_prev = lastEdge;
		}

		lastEdge = currEdge;
		lastNode = currNode;

		++currIndex;
		if (currIndex == ctlPos.size())
			break;
	}

	m_next = this;

	return bStart;
}

void PolyEdge::clear(
	std::stack<PolyEdge*>& polyEdgePool, std::stack<Edge*>& edgePool, bool usePool/* = false*/
	)
{
	PolyEdge *currPE = this, *nextPE;
	do
	{
		std::vector<Edge*> eraseEdges;
		Edge* currEdge = currPE->m_begin;

		if (currPE->m_countEdge == 0)
		{
			do 
			{
				eraseEdges.push_back(currEdge);
				currEdge = currEdge->m_next;
			} while (currEdge);
		}
		else
		{
			for (size_t i = 0; i < currPE->m_countEdge; ++i)
			{
				eraseEdges.push_back(currEdge);;
				currEdge = currEdge->m_next;
			}
		}

		for (size_t i = 0; i < eraseEdges.size(); ++i)
		{
			Edge* e = eraseEdges.at(i);
			if (usePool)
			{
				e->clear();
				edgePool.push(e);
			}
			else
				delete e;
		}

		nextPE = currPE->m_next;
		if (usePool)
		{
			currPE->clear();
			polyEdgePool.push(currPE);
		}
		else
			delete currPE;
		currPE = nextPE;

	} while (nextPE != this);
}

void PolyEdge::clear()
{
	m_begin		= NULL;
	m_bDirty	= false;
	m_next		= this;
	m_type		= COMMON;
}

void PolyEdge::eraseDuplicate(std::stack<PolyEdge*>* polyEdgePool)
{
	PolyEdge* currPolyEdge = this;
	bool bTotErase = false;

	do 
	{
		Edge* currEdge = currPolyEdge->m_begin;
		assert(currEdge);
		bool bLastErase = false;
		bool bFirst = true;

		do 
		{
			bool bErase = currEdge->shouldEraseDuplicated();

			if (bErase && currEdge == currPolyEdge->m_begin)
				currPolyEdge->m_type = ERASE;

			if (bErase != bLastErase)
			{
				if (!bFirst)
				{
					PolyEdge* newPolyEdge;
					if (!polyEdgePool->empty())
					{
						newPolyEdge = polyEdgePool->top(); polyEdgePool->pop();
					}
					else
					{
						newPolyEdge = new PolyEdge;
					}

					currPolyEdge->m_bDirty = true;

					newPolyEdge->m_countEdge = 0;
					newPolyEdge->m_type = bErase ? ERASE : COMMON;
					newPolyEdge->m_pGeoLine = currPolyEdge->m_pGeoLine;
					newPolyEdge->m_bDirty = true;
					newPolyEdge->m_begin = currEdge;
					currEdge->m_prev->m_next = NULL;
					currEdge->m_prev = NULL;

					newPolyEdge->m_next = currPolyEdge->m_next;
					currPolyEdge->m_next = newPolyEdge;

					currPolyEdge = newPolyEdge;
				}
			}

			if (bErase)
			{
				bTotErase = true;
				currEdge->beginNode()->eraseEdge(currEdge, true);
				currEdge->endNode()->eraseEdge(currEdge, false);
			}

			bLastErase = bErase;
			bFirst = false;
			currEdge = currEdge->m_next;
		} while (currEdge != NULL);

		currPolyEdge = currPolyEdge->m_next;
	} while (currPolyEdge != this);

	if (bTotErase)
		updateEdgePointToPolyEdge();
}

void PolyEdge::breakIn(std::stack<PolyEdge*>* polyEdgePool)
{
	PolyEdge* currPolyEdge = this;
	bool bBreakIn = false;

	do 
	{
		if (currPolyEdge->m_type == ERASE)
		{
			currPolyEdge = currPolyEdge->m_next;
			continue;
		}

		Edge* currEdge = currPolyEdge->m_begin;
		assert(currEdge);

		do 
		{
			if (currEdge->m_end->m_type == Node::COMMON)
				;	
			else if (currEdge->m_end->m_type == Node::INTERSECT)
			{
				if (currEdge->m_next == NULL)
					break;

				PolyEdge* newPolyEdge;
				if (!polyEdgePool->empty())
				{
					newPolyEdge = polyEdgePool->top(); polyEdgePool->pop();
				}
				else
				{
					newPolyEdge = new PolyEdge;
				}

				bBreakIn = true;

				newPolyEdge->m_countEdge = 0;
				newPolyEdge->m_type = currPolyEdge->m_type;
				newPolyEdge->m_pGeoLine = NULL;
				newPolyEdge->m_bDirty = true;
				newPolyEdge->m_begin = currEdge->m_next;
				currEdge->m_next->m_prev = NULL;
				currEdge->m_next = NULL;

				newPolyEdge->m_next = currPolyEdge->m_next;
				currPolyEdge->m_next = newPolyEdge;
			}
			else // Node::END
				assert(currEdge->m_next == NULL);

			currEdge = currEdge->m_next;
		} while (currEdge != NULL);

		currPolyEdge = currPolyEdge->m_next;
	} while (currPolyEdge != this);

	if (bBreakIn)
		updateEdgePointToPolyEdge();
}

void PolyEdge::connect()
{
	PolyEdge* currPolyEdge = this;

	do 
	{
		if (currPolyEdge->m_type != COMMON)
		{
			currPolyEdge = currPolyEdge->m_next;
			continue;
		}

		connectOneDirection(currPolyEdge, false, currPolyEdge);
		connectOneDirection(currPolyEdge, true, currPolyEdge);

		currPolyEdge = currPolyEdge->m_next;
	} while (currPolyEdge != this);
}

void PolyEdge::transToLine(std::vector<std::pair<IShape*, bool> >* newData, ISpatialIndex* layer, LineMgr* lineMgr)
{
	PolyEdge* currPolyEdge = this;

	do 
	{
		if (currPolyEdge->m_type != COMMON)
		{
			currPolyEdge = currPolyEdge->m_next;
			continue;
		}

		std::vector<MapPos2D> ctlPos;
		std::vector<PolyEdge*> connectedPE;
		bool bConnected = false;

		Edge* currEdge = currPolyEdge->m_begin;
		
		// forward
		std::vector<MapPos2D> beginPos;
		getConnectPos(currEdge, false, &beginPos, &connectedPE);
		if (!beginPos.empty())
		{
			bConnected = true;
			copy(beginPos.rbegin(), beginPos.rend(), back_inserter(ctlPos));
		}

		// self
		connectedPE.push_back(currPolyEdge);
		ctlPos.push_back(currEdge->beginPos());
		if (currPolyEdge->m_countEdge == 0)
		{
			do 
			{
				ctlPos.push_back(currEdge->endPos());
				if (currEdge->m_next)
					currEdge = currEdge->m_next;
				else
					break;
			} while (true);
		}
		else
		{
			for (size_t i = 0; i < currPolyEdge->m_countEdge; ++i)
			{
				ctlPos.push_back(currEdge->endPos());
				if (i != currPolyEdge->m_countEdge - 1)
					currEdge = currEdge->m_next;
			}
		}

		// back
		size_t oriSize = ctlPos.size();
		getConnectPos(currEdge, true, &ctlPos, &connectedPE);
		if (ctlPos.size() > oriSize)
			bConnected = true;

		GeoLine* newGeoLine;
		createGeoLine(ctlPos, layer, &newGeoLine);

		lineMgr->insertGeoLine(newGeoLine);

		if (connectedPE.size() == 1)
		{
			connectedPE.front()->m_pGeoLine = newGeoLine;
			assert(connectedPE.front()->m_type == COMMON);

			Edge* tmp = connectedPE.front()->m_begin;
			assert(connectedPE.front()->m_countEdge == 0);
			do 
			{
				assert(tmp->m_pPolyEdge = this);
				tmp = tmp->m_next;
			} while (tmp);
		}
		else
		{
			for (size_t i = 0; i < connectedPE.size(); ++i)
			{
				connectedPE.at(i)->m_pGeoLine = newGeoLine;
				connectedPE.at(i)->m_type = CONNECTED;

				Edge* tmp = connectedPE.at(i)->m_begin;
				size_t countEdge = connectedPE.at(i)->m_countEdge;
				assert(countEdge != 0);
				for (size_t i = 0; i < countEdge; ++i)
				{
					tmp->m_pPolyEdge = this;
					tmp = tmp->m_next;
				}
			}
		}

		newData->push_back(std::make_pair(newGeoLine->getShape(), bConnected));

		currPolyEdge = currPolyEdge->m_next;
	} while (currPolyEdge != this);
}

void PolyEdge::updateEdgePointToPolyEdge()
{
	PolyEdge* currPolyEdge = this;

	do 
	{
		currPolyEdge->m_bDirty = true;

		Edge* currEdge = currPolyEdge->m_begin;
		assert(currEdge);

		do 
		{
			currEdge->m_pPolyEdge = currPolyEdge;
			currEdge = currEdge->m_next;
		} while (currEdge != NULL);

		currPolyEdge = currPolyEdge->m_next;

	} while (currPolyEdge != this);
}

void PolyEdge::connectOneDirection(PolyEdge* pe, bool bConnectNext, PolyEdge* original) const
{
	Edge* boundEdge = pe->m_begin;
	if (bConnectNext)
	{
		if (boundEdge->m_pPolyEdge->m_countEdge == 0)
		{
			while (boundEdge->m_next != NULL)
				boundEdge = boundEdge->m_next;
		}
		else
		{
			for (size_t i = 1; i < boundEdge->m_pPolyEdge->m_countEdge; ++i)
				boundEdge = boundEdge->m_next;
		}
	}

	Node* checkedNode;
	std::vector<Edge*> allEdges;
	if (bConnectNext)
		checkedNode = boundEdge->endNode();
	else
		checkedNode = boundEdge->beginNode();

	if (checkedNode->m_type != Node::COMMON)
		return;

	checkedNode->getAllEdges(&allEdges);
	assert(allEdges.size() == 2);

	Edge* checkedEdge;
	if (allEdges.at(0) == boundEdge)
		checkedEdge = allEdges.at(1);
	else if (allEdges.at(1) == boundEdge)
		checkedEdge = allEdges.at(0);
	else
		assert(0);

	if (checkedEdge->m_pPolyEdge->m_type != COMMON)
		return;
	else
		checkedEdge->m_pPolyEdge->m_type = CONNECTED;

	if (boundEdge->m_pPolyEdge->m_countEdge == 0)
		boundEdge->m_pPolyEdge->setCountEdge();

	if (bConnectNext)
		boundEdge->m_next = checkedEdge;
	else
		boundEdge->m_prev = checkedEdge;

	checkedEdge->m_pPolyEdge->setCountEdge();

	if (checkedNode == checkedEdge->beginNode())
	{
		assert(checkedEdge->m_prev == NULL);
		checkedEdge->m_prev = boundEdge;
		connectOneDirection(checkedEdge->m_pPolyEdge, true, original);
	}
	else
	{
		assert(checkedEdge->m_next == NULL);
		checkedEdge->m_next = boundEdge;
		connectOneDirection(checkedEdge->m_pPolyEdge, false, original);
	}
}

void PolyEdge::getConnectPos(const Edge* e, bool beginToEnd, std::vector<MapPos2D>* result,
							 std::vector<PolyEdge*>* connectedPE) const
{
	Edge* checkEdge;

	if (beginToEnd)
	{
		if (e->m_next != NULL)
			checkEdge = e->m_next;
		else
			return;
	}
	else
	{
		if (e->m_prev != NULL)
			checkEdge = e->m_prev;
		else
			return;
	}

	assert(checkEdge->m_pPolyEdge->m_countEdge != 0);
	assert(checkEdge->m_next == e || checkEdge->m_prev == e);

	bool checkBeginToEnd;
	if (checkEdge->m_next == e)
	{
		assert(checkEdge->endNode() == (beginToEnd ? e->endNode() : e->beginNode()));
		checkBeginToEnd = false;
	}
	else if (checkEdge->m_prev == e)
	{
		assert(checkEdge->beginNode() == (beginToEnd ? e->endNode() : e->beginNode()));
		checkBeginToEnd = true;
	}
	else
		assert(0);

	connectedPE->push_back(checkEdge->m_pPolyEdge);

	size_t countCheckEdge = checkEdge->m_pPolyEdge->m_countEdge;
	assert(countCheckEdge != 0);
	for (size_t i = 0; i < countCheckEdge; ++i)
	{
		result->push_back(checkBeginToEnd ? checkEdge->endPos() : checkEdge->beginPos());

		if (i != countCheckEdge - 1)
			checkEdge = checkBeginToEnd ? checkEdge->m_next : checkEdge->m_prev;
	}

	getConnectPos(checkEdge, checkBeginToEnd, result, connectedPE);
}

void PolyEdge::setCountEdge()
{
	size_t count = 0;
	Edge* e = m_begin;

	while (e)
	{
		++count;
		e = e->m_next;
	}

	if (m_countEdge != 0)
		assert(m_countEdge == count);

	m_countEdge = count;
}

void PolyEdge::createGeoLine(const std::vector<MapPos2D>& pos, ISpatialIndex* layer, 
							 GeoLine** geoLine) const
{
	Line* line = new Line(pos);
	*geoLine = new GeoLine(layer, line, m_pGeoLine->m_layerID, m_pGeoLine->m_nodeID, layer->nextObjID(), m_pGeoLine->m_objIndex);
}

//////////////////////////////////////////////////////////////////////////

LineMgr::~LineMgr()
{
	while (!m_polyEdgePool.empty())
	{
		PolyEdge* pe = m_polyEdgePool.top(); m_polyEdgePool.pop();
		delete pe;
	}

	while (!m_edgePool.empty())
	{
		Edge* e = m_edgePool.top(); m_edgePool.pop();
		delete e;
	}
}

void LineMgr::init(const double& precision, const Rect& computeRegion, 
				   const std::vector<ISpatialIndex*>& layers, NodeMgr* pNodeMgr)
{
	m_pNodeMgr = pNodeMgr;

	m_pPoylyEdges.clear();
	for (size_t j = 0; j < layers.size(); ++j)
	{
		SpatialIndex::GetGeoLineVisitor vis(precision, layers.at(j), j, this);
		layers.at(j)->intersectsWithQuery(computeRegion, vis, true);
	}
}

void LineMgr::clear(bool usePool/* = false*/)
{
	for (size_t i = 0; i < m_pGeoLine.size(); ++i)
		delete m_pGeoLine.at(i);

	m_pGeoLine.clear();

	for (size_t i = 0; i < m_pPoylyEdges.size(); ++i)
	{
		PolyEdge* edges = m_pPoylyEdges.at(i);
		if (edges)
			edges->clear(m_polyEdgePool, m_edgePool, usePool);
	}
}

void LineMgr::insertGeoLine(GeoLine* geoLine)
{
	m_pGeoLine.push_back(geoLine);
}

void LineMgr::insertGeoLine(GeoLine* geoLine, const double& precision)
{
	PolyEdge* polyEdge;
	if (!m_polyEdgePool.empty())
	{
		polyEdge = m_polyEdgePool.top(); m_polyEdgePool.pop();
	}
	else
	{
		polyEdge = new PolyEdge;
	}

	if (polyEdge->init(geoLine, m_pNodeMgr, &m_edgePool, precision))
	{
		m_pGeoLine.push_back(geoLine);

		m_pPoylyEdges.push_back(polyEdge);
	}
	else
	{
		delete geoLine;

		polyEdge->clear();
		m_polyEdgePool.push(polyEdge);
	}
}

void LineMgr::eraseDuplicateRoads()
{
	for (size_t i = 0; i < m_pPoylyEdges.size(); ++i)
	{
		PolyEdge* pe = m_pPoylyEdges.at(i);
		pe->eraseDuplicate(&m_polyEdgePool);
	}
}

void LineMgr::breakInRoads()
{
	for (size_t i = 0; i < m_pPoylyEdges.size(); ++i)
	{
		PolyEdge* pe = m_pPoylyEdges.at(i);

		if (pe->m_begin == NULL)
			continue;

		if (pe->m_bDirty)
			pe->breakIn(&m_polyEdgePool);
	}
}

void LineMgr::connectRoads()
{
	for (size_t i = 0; i < m_pPoylyEdges.size(); ++i)
	{
		PolyEdge* pe = m_pPoylyEdges.at(i);

		if (pe->m_begin == NULL)
			continue;

		if (pe->m_bDirty)
			pe->connect();
	}
}

void LineMgr::updateToDatabase(const std::vector<ISpatialIndex*>& layers)
{
	std::vector<std::map<id_type, std::vector<ModifiedLine> > > newData;
	newData.resize(layers.size());

	for (size_t i = 0; i < m_pPoylyEdges.size(); ++i)
	{
		PolyEdge* pe = m_pPoylyEdges.at(i);

		if (pe->m_begin == NULL)
			continue;

		if (pe->m_bDirty)
		{
			size_t layerIndex = m_pGeoLine.at(i)->m_layerID;
			id_type nodeID = m_pGeoLine.at(i)->m_nodeID;

			ModifiedLine ml(pe, layers.at(layerIndex), this, i);
			
			std::map<id_type, std::vector<ModifiedLine> >::iterator itr
				= newData.at(layerIndex).find(nodeID);
			if (itr == newData.at(layerIndex).end())
			{
				std::vector<ModifiedLine> tmp;
				tmp.push_back(ml);
				newData.at(layerIndex).insert(std::make_pair(nodeID, tmp));
			}
			else
			{
				itr->second.push_back(ml);
			}
		}
	}

	for (size_t iLayer = 0; iLayer < newData.size(); ++iLayer)
	{
		SpatialIndex::NVRTree::NVRTree* spatialIndex 
			= dynamic_cast<SpatialIndex::NVRTree::NVRTree*>(layers.at(iLayer));
		if (spatialIndex == NULL)
		{
			assert(0);
			throw Tools::IllegalStateException(
				"LineMgr::updateToDatabase: Should use NVRTree spatial index. "
				);
		}

		std::map<id_type, std::vector<ModifiedLine> >::iterator itrGrid
			= newData.at(iLayer).begin();
		for ( ; itrGrid != newData.at(iLayer).end(); ++itrGrid)
		{
			std::vector<size_t> deleteLines;

			SpatialIndex::NVRTree::NodePtr node = spatialIndex->readNode(itrGrid->first);
			Rect originalMBR = node->getNodeMBR();

			for (size_t iModifiedLine = 0; iModifiedLine < itrGrid->second.size(); ++iModifiedLine)
			{
				ModifiedLine* ml = &itrGrid->second.at(iModifiedLine);
				if (!ml->empty())
					ml->updateToDatabase(node);
				else
					deleteLines.push_back(ml->m_oldIndexInGrid);
			}

			node->deleteRoads(deleteLines);

			if (!node->getNodeMBR().isEmpty() && !originalMBR.containsRect(node->getNodeMBR()))
				spatialIndex->adjustTree(node, originalMBR);

			if (node->getChildrenCount() == 0)
				spatialIndex->condenseTree(node);
			else
				spatialIndex->writeNode(node.get());
		}
	}
}

LineMgr::ModifiedLine::ModifiedLine(PolyEdge* line, ISpatialIndex* index, LineMgr* lineMgr, size_t lineIndex)
{
	m_oldIndexInGrid = lineMgr->m_pGeoLine.at(lineIndex)->m_objIndex;

	if (line->m_bDirty)
		line->transToLine(&m_newData, index, lineMgr);
}

void LineMgr::ModifiedLine::updateToDatabase(SpatialIndex::NVRTree::NodePtr n)
{
	n->updateOneRoadToMuti(m_oldIndexInGrid, m_newData);
}
