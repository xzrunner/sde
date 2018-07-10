#include "MIFAdapter.h"
#include "../Tools/StringTools.h"

using namespace IS_SDE;
using namespace IS_SDE::Adapter;

void MIF_IO::loadMIFHeader(std::ifstream& fin, MIF_Header* header)
{
	std::string sItemName;
	std::string sItemValue;

	std::string textLine;
	getline(fin, textLine);
	textLine = Tools::StringTools::stringTrim(textLine);

	while (_stricmp (textLine.c_str(), "Data") && !fin.eof())
	{
		while( textLine.empty()) 
		{	
			getline(fin, textLine);
			textLine = Tools::StringTools::stringTrim(textLine);
		}

		size_t number = textLine.find_first_of(" ", 0); 
		if (number == -1)
		{
			sItemName  = textLine;
			sItemValue == "";
		}
		else
		{
			sItemName  = textLine.substr(0, number);
			sItemValue = textLine.substr(number+1);
		}

		MIF_Head_Field_enum field = searchField(sItemName);
		switch (field)
		{
		case MIF_DELIMITER:
			header->delimiter = Tools::StringTools::stringTrim(sItemValue);
			break;
		}

		getline(fin, textLine);
		textLine = Tools::StringTools::stringTrim(textLine);
	}
}

void MIF_IO::transFstreamToData(std::ifstream& finMif, /*std::ifstream& finMid, 
								const MIF_Header& header, const std::vector<size_t>& attrIndexes, */
								SpatialIndex::Data** d, id_type id)
{
	MIF_Object_enum type;
	std::string stringTmp;
	finMif >> stringTmp;
	while (finMif.good())
	{
		type = searchType(stringTmp.c_str());
		switch (type)
		{
		case MIF_POINT:
			{
				Point pointTmp;
				loadMIFPoint(finMif, /*finMid, header, attrIndexes, */pointTmp);
				*d = new SpatialIndex::Data(&pointTmp, id);
				return;
			}
		case MIF_LINE:
			{
				Line lineTmp;
				loadMIFLine(finMif, lineTmp);
				*d = new SpatialIndex::Data(&lineTmp, id);
				return;
			}
		case MIF_PLINE:
			{
				finMif >> stringTmp;
				if (!_stricmp(stringTmp.c_str(), "Multiple"))
				{
					// todo!
					size_t multNum;
					finMif >> multNum >> stringTmp;

					Line lineTmp;
					loadMIFPline(finMif, stringTmp, lineTmp);
					*d = new SpatialIndex::Data(&lineTmp, id);

				//	throw Tools::NotSupportedException("MIF_IO::transFstreamToData: Multiple, not support type. ");
				}
				else
				{
					Line lineTmp;
					loadMIFPline(finMif, stringTmp, lineTmp);
					*d = new SpatialIndex::Data(&lineTmp, id);
				}
				return;
			}
		case MIF_REGION:
			{
				Polygon polyTmp;
				loadMIFPoly(finMif, polyTmp);
				*d = new SpatialIndex::Data(&polyTmp, id);
				return;
			}
		case NO_SUCH_OBJECT:
			{
			//	throw Tools::IllegalArgumentException("MIF_IO::transFstreamToData: error type in .mif file. ");
			}
			break;
		default:
			{
				throw Tools::NotSupportedException("MIF_IO::transFstreamToData: not support type in .mif file. ");
			}
		}
		finMif >> stringTmp;
	}
}

MIF_Head_Field_enum MIF_IO::searchField(const std::string& str)
{
	if (!_stricmp(str.c_str(), "Version")) 
		return MIF_VERSION;

	else if (!_stricmp(str.c_str(), "Charset"))
		return MIF_CHARSET;

	else if (!_stricmp(str.c_str(), "Delimiter"))
		return MIF_DELIMITER;

	else if (!_stricmp(str.c_str(), "UNIQUE"))
		return MIF_UNIQUE;

	else if (!_stricmp(str.c_str(), "INDEX"))
		return MIF_INDEX;

	else if (!_stricmp(str.c_str(), "CoordSys"))
		return MIF_COORDSYS;

	else if (!_stricmp(str.c_str(), "TransForm"))
		return MIF_TRANSFORM;

	else if (!_stricmp(str.c_str(), "Columns"))
		return MIF_COLUMNS;

	else
		return NO_SUCH_HEAD_FIELD;
}

MIF_Object_enum MIF_IO::searchType(const std::string& str)
{
	if (!_stricmp(str.c_str(),"Point")) 
		return MIF_POINT;
	else if (!_stricmp(str.c_str(),"Line"))
		return MIF_LINE;
	else if (!_stricmp(str.c_str(),"Pline"))
		return MIF_PLINE;
	else if (!_stricmp(str.c_str(),"Region"))
		return MIF_REGION;
	else if (!_stricmp(str.c_str(), "Text") )
		return MIF_TEXT;
	else if (!_stricmp(str.c_str(), "Arc"))
		return MIF_ARC;
	else 
		return NO_SUCH_OBJECT;
}

void MIF_IO::skipEmptyLines(std::ifstream& fin)
{
	std::string strLine;
	getline(fin, strLine);
	while (Tools::StringTools::stringTrim(strLine).empty() && fin.good())
		getline(fin, strLine);

	fin.seekg(- int(strLine.length()) - 2, std::ios::cur);
}

void MIF_IO::loadMIFPoint(std::ifstream& mifFile, Point& p)
{
	double pos[2];
	mifFile >> pos[0] >> pos[1];
	p.initialize(pos);
	skipEmptyLines(mifFile);
}

void MIF_IO::loadMIFLine(std::ifstream& mifFile, Line& l)
{
	const size_t posNum = 2;
	double pos[posNum * 2];
	for (size_t i = 0; i < posNum; ++i)
		mifFile >> pos[i * 2] >> pos[i * 2 + 1];
	l.initialize(posNum, pos);
	skipEmptyLines(mifFile);
}

void MIF_IO::loadMIFPline(std::ifstream& mifFile, const std::string& s, Line& l)
{
	size_t posNum = atoi(s.c_str());
	if (posNum <= 0)
		throw Tools::IllegalArgumentException("MIF_IO::loadMIFPline: pos size should > 0. ");

	std::vector<double> coords;
	size_t coordsNum = posNum * 2;
	coords.resize(coordsNum, 0.0);
	for (size_t i = 0; i < coordsNum; ++i)
		mifFile >> coords.at(i);

	l.initialize(posNum, &coords.at(0));
	skipEmptyLines(mifFile);
}

void MIF_IO::loadMIFPoly(std::ifstream& mifFile, Polygon& p)
{
	size_t ringNum;
	mifFile >> ringNum;
	if (ringNum <= 0)
		throw Tools::IllegalArgumentException("MIF_IO::loadMIFPoly: ringNum should > 0. ");
	std::vector<size_t> posNum;
	posNum.resize(ringNum, 0);

	std::vector<double> coords;
	size_t endIndex = 0;
	for (size_t i = 0; i < ringNum; ++i)
	{
		size_t beginIndex = endIndex;
		mifFile >> posNum[i];
		if (posNum[i] <= 0)
			throw Tools::IllegalArgumentException("MIF_IO::loadMIFPoly: posNum should > 0. ");
		endIndex += posNum[i] * 2;
		coords.reserve(endIndex);

		double dTmp;
		while (beginIndex++ < endIndex)
		{
			mifFile >> dTmp;
			coords.push_back(dTmp);
		}
	}

	p.initialize(ringNum, &posNum.at(0), &coords.at(0));
	skipEmptyLines(mifFile);
}