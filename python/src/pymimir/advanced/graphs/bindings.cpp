#include "bindings.hpp"

#include "../init_declarations.hpp"

#include <nanobind/trampoline.h>

namespace mimir::graphs
{

void bind_module_definitions(nb::module_& m)
{
    bind_vertex<PyVertex>(m, PyVertexProperties<PyVertex>::name);
    bind_edge<PyEdge>(m, PyEdgeProperties<PyEdge>::name);
    bind_static_graph<PyVertex, PyEdge>(m, "StaticPyGraph");
    bind_dynamic_graph<PyVertex, PyEdge>(m, "DynamicPyGraph");

    nb::class_<color_refinement::CertificateImpl>(m, "CertificateColorRefinement")
        .def("__str__", [](const color_refinement::CertificateImpl& self) { return to_string(self); })
        .def("__repr__", [](const color_refinement::CertificateImpl& self) { return to_string(self); })
        .def("__eq__", [](const color_refinement::CertificateImpl& lhs, const color_refinement::CertificateImpl& rhs) { return lhs == rhs; })
        .def("__ne__", [](const color_refinement::CertificateImpl& lhs, const color_refinement::CertificateImpl& rhs) { return lhs != rhs; })
        .def("__hash__", [](const color_refinement::CertificateImpl& self) { return loki::Hash<color_refinement::CertificateImpl>()(self); })
        .def("get_canonical_color_compression_function", &color_refinement::CertificateImpl::get_canonical_color_compression_function)
        .def("get_canonical_configuration_compression_function",
             [](const color_refinement::CertificateImpl& self)
             {
                 auto result = nb::dict {};
                 for (const auto& [configuration, hash] : self.get_canonical_configuration_compression_function())
                 {
                     auto list = nb::list();
                     for (const auto& element : configuration.second)
                     {
                         list.append(nb::int_(element));
                     }
                     auto key = nb::make_tuple(nb::int_(configuration.first), nb::tuple(list));
                     result[key] = hash;
                 }
                 return result;
             })
        .def("get_hash_to_color", &color_refinement::CertificateImpl::get_hash_to_color);
    nb::class_<kfwl::IsomorphismTypeCompressionFunction>(m, "KFWLIsomorphismTypeCompressionFunction")  //
        .def(nb::init<>());
    // bind_static_graph instantiates certificate computation for 2,3,4
    bind_kfwl_certificate<2>(m, "Certficate2FWL");
    bind_kfwl_certificate<3>(m, "Certficate3FWL");
    bind_kfwl_certificate<4>(m, "Certficate4FWL");

    nb::class_<IProperty>(m, "IProperty")
        .def("__str__", &IProperty::str)
        .def("__repr__", [](const IProperty& p) { return p.str(); })
        .def("__hash__", [](const IProperty& p) { return (nb::ssize_t) p.hash(); })
        .def("__eq__", [](const IProperty& a, const IProperty& b) { return a == b; })
        .def("__lt__", [](const IProperty& a, const IProperty& b) { return a < b; });

    nb::class_<PyProperty, IProperty>(m, "PyProperty")
        .def(nb::init<nb::object>(), "obj"_a, "Wrap any Python object as a runtime property")
        .def("__str__", &PyProperty::str)
        .def("__repr__", [](const PyProperty& p) { return p.str(); })
        .def("__hash__", [](const PyProperty& p) { return (nb::ssize_t) p.hash(); })
        .def(
            "__eq__",
            [](const PyProperty& a, const IProperty& b) { return a == b; },
            nb::is_operator())
        .def("__lt__", [](const PyProperty& a, const IProperty& b) { return a < b; }, nb::is_operator());

    nb::class_<PropertyValue>(m, "PropertyValue")
        .def(nb::init<std::shared_ptr<IProperty>>(), "impl"_a)
        .def("__str__", &PropertyValue::str)
        .def("__repr__", [](const PropertyValue& v) { return v.str(); })
        .def("__hash__", [](const PropertyValue& v) { return v.hash(); })
        .def("__eq__", &PropertyValue::operator==, nb::is_operator())
        .def("__lt__", &PropertyValue::operator<, nb::is_operator());

    nb::class_<nauty::SparseGraph>(m, "NautySparseGraph")
        .def(nb::init<const StaticGraph<PyVertex, PyEdge>&>())
        .def(nb::init<const StaticForwardGraph<StaticGraph<PyVertex, PyEdge>>&>())
        .def(nb::init<const StaticBidirectionalGraph<StaticGraph<PyVertex, PyEdge>>&>())
        .def("__str__", [](const nauty::SparseGraph& self) { return to_string(self); })
        .def("__repr__", [](const nauty::SparseGraph& self) { return to_string(self); })
        .def("__eq__", [](const nauty::SparseGraph& lhs, const nauty::SparseGraph& rhs) { return loki::EqualTo<nauty::SparseGraph>()(lhs, rhs); })
        .def("__ne__", [](const nauty::SparseGraph& lhs, const nauty::SparseGraph& rhs) { return !loki::EqualTo<nauty::SparseGraph>()(lhs, rhs); })
        .def("__hash__", [](const nauty::SparseGraph& self) { return loki::Hash<nauty::SparseGraph>()(self); })
        .def("canonize", &nauty::SparseGraph::canonize);
}

}
