
#ifndef MIMIR_PYTHON_DATASETS_BINDINGS_HPP
#define MIMIR_PYTHON_DATASETS_BINDINGS_HPP

#include "../graphs/bindings.hpp"
#include "../init_declarations.hpp"

namespace mimir::graphs
{

template<>
struct PyVertexProperties<ProblemVertex>
{
    static constexpr const char* name = "ProblemVertex";
};

template<>
struct PyVertexProperties<ClassVertex>
{
    static constexpr const char* name = "ClassVertex";
};

template<>
struct PyVertexProperties<TupleGraphVertex>
{
    static constexpr const char* name = "TupleGraphVertex";
};

template<>
struct PyEdgeProperties<ProblemEdge>
{
    static constexpr const char* name = "ProblemEdge";
};

template<>
struct PyEdgeProperties<ClassEdge>
{
    static constexpr const char* name = "ClassEdge";
};

}

#endif