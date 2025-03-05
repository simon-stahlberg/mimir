
#ifndef MIMIR_DATASETS_BINDINGS_HPP
#define MIMIR_DATASETS_BINDINGS_HPP

#include "../graphs/bindings.hpp"
#include "init_declarations.hpp"
#include "mimir/graphs.hpp"

namespace mm = mimir;
namespace py = pybind11;

template<>
struct PyVertexProperties<mm::ProblemVertex>
{
    static constexpr std::string name = "ProblemVertex";
};

template<>
struct PyVertexProperties<mm::ClassVertex>
{
    static constexpr std::string name = "ClassVertex";
};

template<>
struct PyEdgeProperties<mm::ProblemEdge>
{
    static constexpr std::string name = "ProblemEdge";
};

template<>
struct PyEdgeProperties<mm::ClassEdge>
{
    static constexpr std::string name = "ClassEdge";
};

#endif