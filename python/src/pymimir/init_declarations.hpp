
#ifndef MIMIR_INIT_DECLARATIONS_HPP
#define MIMIR_INIT_DECLARATIONS_HPP

#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/utils/utils.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors
#include <pybind11/stl_bind.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#include "mimir/mimir.hpp"

//
// utilities
//

struct default_elem_repr
{
    template<typename T>
    std::string operator()(const T& elem) const
    {
        return mimir::deref(elem).str();
    }
};

template<typename BoundVectorType, typename ReprFunctor = default_elem_repr>
inline auto& def_opaque_vector_repr(auto& cls, const std::string& class_name, ReprFunctor elem_repr = ReprFunctor {})
{
    return cls;
}

template<typename BoundVectorType, typename ReprFunctor = default_elem_repr>
    requires requires(typename BoundVectorType::value_type elem) {
        { mimir::deref(elem).str() } -> std::convertible_to<std::string>;
    }
inline auto& def_opaque_vector_repr(auto& cls, const std::string& class_name, ReprFunctor elem_repr = ReprFunctor {})
{
    return cls.def("__str__",
                   [=](const BoundVectorType& self) { return fmt::format("{}[{}]", class_name, fmt::join(std::views::transform(self, elem_repr), ",")); });
}

//
// init - declarations:
//
void init_pymimir(pybind11::module_& m);

#endif  // MIMIR_INIT_DECLARATIONS_HPP
