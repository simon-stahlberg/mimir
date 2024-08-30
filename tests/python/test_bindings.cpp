#include "fixture.hpp"

#include <gtest/gtest.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

constexpr std::string_view project_dir = "../../../";

TEST_F(PymimirFixture, get_ground_atoms)
{
    auto parser =
        pymimir().attr("PDDLParser")(std::string(project_dir) + "data/blocks_4/domain.pddl", std::string(project_dir) + "data/blocks_4/test_problem.pddl");
    auto factory = parser.attr("get_pddl_factories")();
    auto static_atoms = factory.attr("get_static_ground_atoms")();
    EXPECT_EQ(py::len(static_atoms), 0);
    auto derived_atoms = factory.attr("get_derived_ground_atoms")();
    EXPECT_EQ(py::len(derived_atoms), 0);
    auto fluent_atoms_from_ind = factory.attr("get_fluent_ground_atoms_from_indices")(py::eval("range")(0, 8));
    EXPECT_EQ(py::len(fluent_atoms_from_ind), 8);
    auto fluent_atoms = factory.attr("get_fluent_ground_atoms")();
    EXPECT_EQ(py::len(fluent_atoms), 8);
    auto atoms = factory.attr("get_ground_atoms")();
    EXPECT_GE(py::len(atoms), py::len(fluent_atoms) + py::len(derived_atoms) + py::len(static_atoms));
}

TEST_F(PymimirFixture, parser_factory_atom_lifetime)
{
    auto parser =
        pymimir().attr("PDDLParser")(std::string(project_dir) + "data/blocks_4/domain.pddl", std::string(project_dir) + "data/blocks_4/test_problem.pddl");
    auto factory = parser.attr("get_pddl_factories")();
    auto atoms = factory.attr("get_ground_atoms")();
    auto& internals = py::detail::get_internals();
    for (auto atom : py::cast<py::list>(atoms))
    {
        const auto& patients = internals.patients[atom.ptr()];
        EXPECT_TRUE(std::find(patients.begin(), patients.end(), factory.ptr()) != patients.end());
    }
}

TEST_F(PymimirFixture, test_list_repr)
{
    auto parser =
        pymimir().attr("PDDLParser")(std::string(project_dir) + "data/blocks_4/domain.pddl", std::string(project_dir) + "data/blocks_4/test_problem.pddl");
    auto factory = parser.attr("get_pddl_factories")();
    auto atoms = factory.attr("get_fluent_ground_atoms")();
    auto str = py::str(atoms).cast<std::string>();
#ifndef NDEBUG
    fmt::println("{}", str);
#endif
    EXPECT_TRUE(str.find("FluentGroundAtomList") != std::string::npos);
    for (auto atom : py::cast<py::list>(atoms))
    {
        auto atom_str = py::str(atom).cast<std::string>();
#ifndef NDEBUG
        fmt::println("{}", atom_str);
#endif
        EXPECT_TRUE(str.find(atom_str) != std::string::npos);
    }
}