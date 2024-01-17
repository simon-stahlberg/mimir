#ifndef MIMIR_FORMALISM_FACTORIES_HPP_
#define MIMIR_FORMALISM_FACTORIES_HPP_

#include "type.hpp"
#include "problem.hpp"

#include <loki/common/pddl/persistent_factory.hpp>

namespace mimir {

using TypeFactory =               loki::PersistentFactory<Type, 1000>;
//using VariableFactory =           loki::PersistentFactory<pddl::VariableImpl, 1000>;
//using TermFactory =               loki::PersistentFactory<pddl::TermImpl, 1000>;
//using ObjectFactory =             loki::PersistentFactory<pddl::ObjectImpl, 1000>;
//using AtomFactory =               loki::PersistentFactory<pddl::AtomImpl, 1000>;
//using LiteralFactory =            loki::PersistentFactory<pddl::LiteralImpl, 1000>;
//using ParameterFactory =          loki::PersistentFactory<pddl::ParameterImpl, 1000>;
//using PredicateFactory =          loki::PersistentFactory<pddl::PredicateImpl, 1000>;
//using FunctionExpressionFactory = loki::PersistentFactory<pddl::FunctionExpressionImpl, 1000>;
//using FunctionFactory =           loki::PersistentFactory<pddl::FunctionImpl, 1000>;
//using FunctionSkeletonFactory =   loki::PersistentFactory<pddl::FunctionSkeletonImpl, 1000>;
//using ConditionFactory =          loki::PersistentFactory<pddl::ConditionImpl, 1000>;
//using EffectFactory =             loki::PersistentFactory<pddl::EffectImpl, 1000>;
//using ActionFactory =             loki::PersistentFactory<pddl::ActionImpl, 100>;
//using DerivedPredicateFactory =   loki::PersistentFactory<pddl::DerivedPredicateImpl, 100>;
//using OptimizationMetricFactory = loki::PersistentFactory<pddl::OptimizationMetricImpl, 100>;
//using NumericFluentFactory =      loki::PersistentFactory<pddl::NumericFluentImpl, 1000>;
//using DomainFactory =             loki::PersistentFactory<pddl::DomainImpl, 1>;
using ProblemFactory =            loki::PersistentFactory<Problem, 10>;


}

#endif