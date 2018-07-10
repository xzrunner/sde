#include "DensityConsistency.h"
#include "../../../Algorithms/ComputationalGeometry/VoronoiByDelaunay.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Measure::Points;

DensityConsistency::DensityConsistency(IAgent* agent)
	: m_result(-1)
{
	fetchAreaList(agent, m_srcAreaList);
}

void DensityConsistency::getMeasureValue(Generalization::IAgent* agent, Tools::PropertySet& ps)
{
	Tools::Variant var;

	if (m_result == -1)
		m_result = calDensityConsistencyVal(agent);
	
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_result;

	ps.setProperty("DensityConsistency", var);
}

void DensityConsistency::fetchAreaList(IAgent* agent, std::vector<double>& areaList)
{
	Tools::Variant var;
	agent->getAgentProperty("SortedVoronoiArea", var);
	if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("DensityConsistency::fetchAreaList: Didn't find property.");
	byte* srcAreaList = static_cast<byte*>(var.m_val.pvVal);

	size_t count = 0;
	memcpy(&count, srcAreaList, sizeof(size_t));
	srcAreaList += sizeof(size_t);
	areaList.reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		double area = 0;
		memcpy(&area, srcAreaList, sizeof(double));
		srcAreaList += sizeof(double);
		areaList.push_back(area);
	}
}

double DensityConsistency::calDensityConsistencyVal(IAgent* agent)
{
	std::vector<double> currAreaList;
	fetchAreaList(agent, currAreaList);

	std::vector<double> cmpDensity;
	size_t cmpNum = std::min(m_srcAreaList.size(), currAreaList.size());
	cmpDensity.reserve(cmpNum);
	for (size_t i = 0; i < cmpNum; ++i)
		cmpDensity.push_back(m_srcAreaList[i] / currAreaList[i]);

	double ret = calStandardDeviation(cmpDensity);
	assert(ret != -1);
	return ret;
}

double DensityConsistency::calStandardDeviation(const std::vector<double>& val) const
{
	if (val.empty())
		return -1;

	size_t num = val.size();

	double even = 0.0;
	for (size_t i = 0; i < num; ++i)
		even += val[i];
	even /= val.size();

	double sum = 0;
	for (size_t i = 0; i < num; ++i)
		sum += (val[i] - even) * (val[i] - even);
	return sqrt(sum / num);
}