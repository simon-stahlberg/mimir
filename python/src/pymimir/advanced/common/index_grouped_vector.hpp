
#ifndef MIMIR_PYTHON_COMMON_INDEX_GROUPED_VECTORS_HPP
#define MIMIR_PYTHON_COMMON_INDEX_GROUPED_VECTORS_HPP

#include "../init_declarations.hpp"

using namespace mimir;

/// @brief Binds a IndexGroupedVector as an unmodifiable python object.
/// Modifiable IndexGroupedVector are more complicated because they use std::span.
/// See section regarding bind_const_span.
template<typename T>
nb::class_<IndexGroupedVector<T>, holder_type> bind_const_index_grouped_vector(nb::handle m, const std::string& name)
{
    using IndexGroupedVectorType = IndexGroupedVector<T>;

    nb::class_<IndexGroupedVector<T>, holder_type> cl(m, name.c_str());

    using SizeType = typename IndexGroupedVectorType::size_type;
    using DiffType = typename IndexGroupedVectorType::difference_type;
    using ItType = typename IndexGroupedVectorType::const_iterator;

    auto wrap_i = [](DiffType i, SizeType n)
    {
        if (i < 0)
        {
            i += n;
        }
        if (i < 0 || (SizeType) i >= n)
        {
            throw nb::index_error();
        }
        return i;
    };

    /**
     * Accessors
     */

    /* This requires std::vector<T> to be an opague type
       because weak references to python lists are not allowed.
    */
    cl.def(
        "__getitem__",
        [wrap_i](const IndexGroupedVector& v, DiffType i) -> T
        {
            i = wrap_i(i, v.size());
            return v[(SizeType) i];
        },
        nb::rv_policy::copy,
        nb::keep_alive<0, 1>()); /* Essential keep span alive while copy of element exists */

    cl.def(
        "__iter__",
        [](const IndexGroupedVector& v)
        {
            return nb::make_iterator<nb::rv_policy::copy, ItType, ItType, T>(
                v.begin(),
                v.end(),
                nb::rv_policy::copy,
                nb::keep_alive<0, 1>()); /* Essential: keep iterator alive while copy of element exists. */
        },
        nb::keep_alive<0, 1>() /* Essential: keep index grouped vector alive while iterator exists */
    );

    cl.def("__len__", &IndexGroupedVector::size);

    return cl;
}

#endif