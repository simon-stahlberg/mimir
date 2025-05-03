#include "bindings.hpp"

#include "../init_declarations.hpp"

#include <nanobind/trampoline.h>

namespace mimir::graphs
{

class PyColor : public IColor
{
public:
    PyColor(nb::args args) : m_tuple(nb::tuple(args)) {}

    bool operator==(const IColor& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const PyColor&>(other);

            return m_tuple.equal(other_derived.m_tuple);
        }
        return false;
    }
    bool operator<(const IColor& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const PyColor&>(other);

            return bool(nb::bool_(m_tuple.attr("__lt__")(other_derived.m_tuple)));
        }
        return typeid(*this).before(typeid(other));
    }
    std::string str() const override { return nb::str(m_tuple).c_str(); }
    size_t hash() const override { return nb::hash(m_tuple); }

private:
    nb::tuple m_tuple;
};

void bind_module_definitions(nb::module_& m)
{
    bind_vertex<PyVertex>(m, PyVertexProperties<PyVertex>::name);
    bind_vertex<EmptyVertex>(m, PyVertexProperties<EmptyVertex>::name);
    bind_vertex<ColoredVertex>(m, PyVertexProperties<ColoredVertex>::name);
    bind_edge<PyEdge>(m, PyEdgeProperties<PyEdge>::name);
    bind_edge<EmptyEdge>(m, PyEdgeProperties<EmptyEdge>::name);
    bind_edge<ColoredEdge>(m, PyEdgeProperties<ColoredEdge>::name);
    bind_static_graph<PyVertex, PyEdge>(m, "StaticPyGraph");
    bind_static_graph<EmptyVertex, EmptyEdge>(m, "StaticEmptyGraph");
    bind_static_graph<ColoredVertex, EmptyEdge>(m, "StaticVertexColoredGraph");
    bind_static_graph<ColoredVertex, ColoredEdge>(m, "StaticEdgeColoredGraph");
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

    nb::class_<Color>(m, "Color")  //
        .def(nb::new_([](nb::args args) { return Color(std::shared_ptr<IColor>(std::make_shared<PyColor>(args))); }))
        .def("__str__", &Color::str)
        .def("__eq__", &Color::operator==)
        .def("__lt__", &Color::operator<)
        .def("__hash__", &Color::hash);

    nb::class_<nauty::SparseGraph>(m, "NautySparseGraph")
        .def(nb::init<const StaticGraph<ColoredVertex, EmptyEdge>&>())
        .def(nb::init<const StaticForwardGraph<StaticGraph<ColoredVertex, EmptyEdge>>&>())
        .def(nb::init<const StaticBidirectionalGraph<StaticGraph<ColoredVertex, EmptyEdge>>&>())
        .def("__str__", [](const nauty::SparseGraph& self) { return to_string(self); })
        .def("__repr__", [](const nauty::SparseGraph& self) { return to_string(self); })
        .def("__eq__", [](const nauty::SparseGraph& lhs, const nauty::SparseGraph& rhs) { return loki::EqualTo<nauty::SparseGraph>()(lhs, rhs); })
        .def("__ne__", [](const nauty::SparseGraph& lhs, const nauty::SparseGraph& rhs) { return !loki::EqualTo<nauty::SparseGraph>()(lhs, rhs); })
        .def("__hash__", [](const nauty::SparseGraph& self) { return loki::Hash<nauty::SparseGraph>()(self); })
        .def("canonize", &nauty::SparseGraph::canonize);
}

}
