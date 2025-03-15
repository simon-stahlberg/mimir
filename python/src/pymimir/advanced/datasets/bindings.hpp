
#ifndef MIMIR_PYTHON_DATASETS_BINDINGS_HPP
#define MIMIR_PYTHON_DATASETS_BINDINGS_HPP

#include "../graphs/bindings.hpp"
#include "../init_declarations.hpp"

namespace mimir::bindings
{

template<>
struct PyVertexProperties<mm::graphs::ProblemVertex>
{
    static constexpr std::string name = "ProblemVertex";
};

template<>
struct PyVertexProperties<mm::graphs::ClassVertex>
{
    static constexpr std::string name = "ClassVertex";
};

template<>
struct PyEdgeProperties<mm::graphs::ProblemEdge>
{
    static constexpr std::string name = "ProblemEdge";
};

template<>
struct PyEdgeProperties<mm::graphs::ClassEdge>
{
    static constexpr std::string name = "ClassEdge";
};

}

#endif