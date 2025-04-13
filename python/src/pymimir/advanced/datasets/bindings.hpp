
#ifndef MIMIR_PYTHON_DATASETS_BINDINGS_HPP
#define MIMIR_PYTHON_DATASETS_BINDINGS_HPP

#include "../graphs/bindings.hpp"
#include "../init_declarations.hpp"

namespace mimir::bindings
{

template<>
struct PyVertexProperties<mm::graphs::ProblemVertex>
{
    static constexpr const char* name = "ProblemVertex";
};

template<>
struct PyVertexProperties<mm::graphs::ClassVertex>
{
    static constexpr const char* name = "ClassVertex";
};

template<>
struct PyVertexProperties<mm::graphs::TupleGraphVertex>
{
    static constexpr const char* name = "TupleGraphVertex";
};

template<>
struct PyEdgeProperties<mm::graphs::ProblemEdge>
{
    static constexpr const char* name = "ProblemEdge";
};

template<>
struct PyEdgeProperties<mm::graphs::ClassEdge>
{
    static constexpr const char* name = "ClassEdge";
};

}

#endif