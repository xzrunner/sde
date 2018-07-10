#ifndef _IS_SDE_GENERALIZATION_BASE_H_
#define _IS_SDE_GENERALIZATION_BASE_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		enum OperatorType
		{
			// Points
			POITNS_AGGREGATION,
			POINTS_SELECTIVE,
			POITNS_REGIONIZATION,
			POINTS_SIMPLIFICATION,
			// Line
			LINE_POINT_REDUCTION,
			// Lines
			LINES_SELECTIVE

		}; // OperatorType

		//
		// Interfaces
		//

		interface IAlgorithm
		{
		public:
			// todo: don't use IShape, only use MapPos
			virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest) = 0;
			virtual OperatorType type() const = 0;
			virtual ~IAlgorithm() {}
		}; // IAlgorithm

		interface IConstraint
		{
		public:
			virtual size_t evaluateConstraint() const = 0;
			virtual size_t getImportance() const = 0;
			virtual size_t getPriority() const = 0;
			virtual void proposePlans(std::vector<IAlgorithm*>& plans) const = 0;
			virtual void clearBuffer() = 0;
			virtual ~IConstraint() {}
		}; // IConstraint

		interface IAgent : public Tools::ISerializable
		{
		public:
			virtual void getAgentProperty(const std::string& property, Tools::Variant& var) const = 0;
			virtual void initConstraints(const std::vector<IConstraint*>& cons) = 0;
			virtual void proposePlans(std::vector<IAlgorithm*>& plans) const = 0;
			virtual void triggerBestPlan(IAlgorithm* plan) = 0;
			virtual double calHappiness() const = 0;
			virtual bool doNextOperation() = 0;
			virtual ~IAgent() {}
		}; // IAgent

		interface IClustering
		{
		public:
			virtual void clustering(const std::vector<IShape*>& src, std::vector<std::vector<IShape*> >& dest) = 0;
			virtual ~IClustering() {}
		}; // IClustering

		interface IOperator
		{
		public:
			//virtual void implement() = 0;
			//virtual const std::vector<IShape*>& getResult() const = 0;
			virtual ~IOperator() {}
		}; // IOperator

		interface IMeasure
		{
		public:
			virtual void getMeasureValue(IAgent* agent, Tools::PropertySet& ps) = 0;
			virtual ~IMeasure() {}
		}; // IMeasure
	}
}

#endif // _IS_SDE_GENERALIZATION_BASE_H_