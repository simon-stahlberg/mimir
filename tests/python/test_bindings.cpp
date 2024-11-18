#include "fixture.hpp"

#include <gtest/gtest.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

constexpr std::string_view project_dir = "../../../";

TEST_F(PymimirFixture, get_ground_atoms)
{
    auto parser =
        pymimir().attr("PDDLParser")(std::string(project_dir) + "data/blocks_4/domain.pddl", std::string(project_dir) + "data/blocks_4/test_problem.pddl");
    auto factory = parser.attr("get_pddl_repositories")();
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

TEST_F(PymimirFixture, get_predicates)
{
    auto parser =
        pymimir().attr("PDDLParser")(std::string(project_dir) + "data/delivery/domain.pddl", std::string(project_dir) + "data/delivery/test_problem.pddl");
    auto domain = parser.attr("get_domain")();
    auto static_predicates = domain.attr("get_static_predicates")();
    EXPECT_EQ(py::len(static_predicates), 5);
    auto derived_predicates = domain.attr("get_derived_predicates")();
    EXPECT_EQ(py::len(derived_predicates), 0);
    auto fluent_predicates = domain.attr("get_fluent_predicates")();
    EXPECT_EQ(py::len(fluent_predicates), 3);
    auto predicates = domain.attr("get_predicates")();
    EXPECT_GE(py::len(predicates), py::len(fluent_predicates) + py::len(derived_predicates) + py::len(static_predicates));
}

TEST_F(PymimirFixture, get_goal_condition)
{
    auto parser =
        pymimir().attr("PDDLParser")(std::string(project_dir) + "data/delivery/domain.pddl", std::string(project_dir) + "data/delivery/test_problem.pddl");
    auto problem = parser.attr("get_problem")();
    auto static_goals = problem.attr("get_static_goal_condition")();
    EXPECT_EQ(py::len(static_goals), 0);
    auto derived_goals = problem.attr("get_derived_goal_condition")();
    EXPECT_EQ(py::len(derived_goals), 0);
    auto fluent_goals = problem.attr("get_fluent_goal_condition")();
    EXPECT_EQ(py::len(fluent_goals), 1);
    auto goals = problem.attr("get_goal_condition")();
    EXPECT_GE(py::len(goals), py::len(fluent_goals) + py::len(derived_goals) + py::len(static_goals));
}

TEST_F(PymimirFixture, parser_factory_atom_lifetime)
{
    auto parser =
        pymimir().attr("PDDLParser")(std::string(project_dir) + "data/blocks_4/domain.pddl", std::string(project_dir) + "data/blocks_4/test_problem.pddl");
    auto factory = parser.attr("get_pddl_repositories")();
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
    auto factory = parser.attr("get_pddl_repositories")();
    auto atoms = factory.attr("get_fluent_ground_atoms")();
    auto str = py::str(atoms).cast<std::string>();

    EXPECT_TRUE(str.find("FluentGroundAtomList") != std::string::npos);
    for (auto atom : py::cast<py::list>(atoms))
    {
        auto atom_str = py::str(atom).cast<std::string>();
        EXPECT_TRUE(str.find(atom_str) != std::string::npos);
    }
}