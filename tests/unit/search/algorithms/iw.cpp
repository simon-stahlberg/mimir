#include "mimir/formalism/formalism.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/plan.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

/// @brief Instantiate a lifted IW search
class LiftedIWPlanner
{
private:
    PDDLParser m_parser;

    std::shared_ptr<ILiftedAAGEventHandler> m_aag_event_handler;
    std::shared_ptr<LiftedAAG> m_aag;
    std::shared_ptr<SuccessorStateGenerator> m_ssg;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    LiftedIWPlanner(const fs::path& domain_file, const fs::path& problem_file, int arity) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_aag_event_handler(std::make_shared<DefaultLiftedAAGEventHandler>()),
        m_aag(std::make_shared<LiftedAAG>(m_parser.get_problem(), m_parser.get_factories(), m_aag_event_handler)),
        m_ssg(std::make_shared<SuccessorStateGenerator>(m_aag)),
        m_brfs_event_handler(std::make_shared<DefaultBrFSAlgorithmEventHandler>()),
        m_iw_event_handler(std::make_shared<DefaultIWAlgorithmEventHandler>()),
        m_algorithm(std::make_unique<IWAlgorithm>(m_aag, arity, m_ssg, m_brfs_event_handler, m_iw_event_handler))
    {
    }

    std::tuple<SearchStatus, Plan> find_solution()
    {
        auto action_view_list = GroundActionList {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }

    const IWAlgorithmStatistics& get_iw_statistics() const { return m_iw_event_handler->get_statistics(); }
    const LiftedAAGStatistics& get_aag_statistics() const { return m_aag_event_handler->get_statistics(); }
};

/// @brief Instantiate a grounded IW search
class GroundedIWPlanner
{
private:
    PDDLParser m_parser;

    std::shared_ptr<IGroundedAAGEventHandler> m_aag_event_handler;
    std::shared_ptr<GroundedAAG> m_aag;
    std::shared_ptr<SuccessorStateGenerator> m_ssg;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    GroundedIWPlanner(const fs::path& domain_file, const fs::path& problem_file, int arity) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_aag_event_handler(std::make_shared<DefaultGroundedAAGEventHandler>()),
        m_aag(std::make_shared<GroundedAAG>(m_parser.get_problem(), m_parser.get_factories(), m_aag_event_handler)),
        m_ssg(std::make_shared<SuccessorStateGenerator>(m_aag)),
        m_brfs_event_handler(std::make_shared<DefaultBrFSAlgorithmEventHandler>()),
        m_iw_event_handler(std::make_shared<DefaultIWAlgorithmEventHandler>()),
        m_algorithm(std::make_unique<IWAlgorithm>(m_aag, arity, m_ssg, m_brfs_event_handler, m_iw_event_handler))
    {
    }

    std::tuple<SearchStatus, Plan> find_solution()
    {
        auto action_view_list = GroundActionList {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }

    const IWAlgorithmStatistics& get_iw_statistics() const { return m_iw_event_handler->get_statistics(); }
    const GroundedAAGStatistics& get_aag_statistics() const { return m_aag_event_handler->get_statistics(); }
};

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth0Test)
{
    const int arity = 0;
    const int num_atoms = 3;

    const auto atom_index_mapper = std::make_shared<FluentAndDerivedMapper>();
    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StateTupleIndexGenerator(atom_index_mapper, tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("()", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth1Test)
{
    const int arity = 1;
    const int num_atoms = 3;

    const auto atom_index_mapper = std::make_shared<FluentAndDerivedMapper>();
    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StateTupleIndexGenerator(atom_index_mapper, tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("(0,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth2Test)
{
    const int arity = 2;
    const int num_atoms = 3;

    const auto atom_index_mapper = std::make_shared<FluentAndDerivedMapper>();
    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StateTupleIndexGenerator(atom_index_mapper, tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("(0,2,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(0,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth1Test)
{
    const int arity = 1;
    const int num_atoms = 4;

    const auto atom_index_mapper = std::make_shared<FluentAndDerivedMapper>();
    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(atom_index_mapper, tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        1,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(1,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test1)
{
    const int arity = 2;
    const int num_atoms = 4;

    const auto atom_index_mapper = std::make_shared<FluentAndDerivedMapper>();
    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(atom_index_mapper, tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        1,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(1,2,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(1,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,1,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test2)
{
    const int arity = 3;
    const int num_atoms = 64;

    const auto atom_index_mapper = std::make_shared<FluentAndDerivedMapper>();
    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(atom_index_mapper, tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        3,
        4,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        6,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(6,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(0,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,2,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test3)
{
    const int arity = 2;
    const int num_atoms = 64;

    const auto atom_index_mapper = std::make_shared<FluentAndDerivedMapper>();
    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(atom_index_mapper, tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        1,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        2,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(2,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

/**
 * Delivery
 */

TEST(MimirTests, SearchAlgorithmsIWGroundedDeliveryTest)
{
    auto iw = GroundedIWPlanner(fs::path(std::string(DATA_DIR) + "delivery/domain.pddl"), fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl"), 3);
    const auto [search_status, plan] = iw.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);

    const auto& aag_statistics = iw.get_aag_statistics();

    EXPECT_EQ(aag_statistics.get_num_delete_free_reachable_fluent_ground_atoms(), 10);
    EXPECT_EQ(aag_statistics.get_num_delete_free_reachable_derived_ground_atoms(), 0);
    EXPECT_EQ(aag_statistics.get_num_delete_free_actions(), 16);
    EXPECT_EQ(aag_statistics.get_num_delete_free_axioms(), 0);

    EXPECT_EQ(aag_statistics.get_num_ground_actions(), 16);
    EXPECT_EQ(aag_statistics.get_num_nodes_in_action_match_tree(), 32);

    EXPECT_EQ(aag_statistics.get_num_ground_axioms(), 0);
    EXPECT_EQ(aag_statistics.get_num_nodes_in_axiom_match_tree(), 1);

    const auto& iw_statistics = iw.get_iw_statistics();

    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_generated_until_f_value().back(), 18);
    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_expanded_until_f_value().back(), 7);
    EXPECT_EQ(iw_statistics.get_effective_width(), 2);
}

TEST(MimirTests, SearchAlgorithmsIWLiftedDeliveryTest)
{
    auto iw = LiftedIWPlanner(fs::path(std::string(DATA_DIR) + "delivery/domain.pddl"), fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl"), 3);
    const auto [search_status, plan] = iw.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);

    const auto& aag_statistics = iw.get_aag_statistics();

    EXPECT_EQ(aag_statistics.get_num_ground_action_cache_hits_until_f_value().back(), 25);
    EXPECT_EQ(aag_statistics.get_num_ground_action_cache_misses_until_f_value().back(), 12);

    EXPECT_EQ(aag_statistics.get_num_ground_axiom_cache_hits_until_f_value().back(), 0);
    EXPECT_EQ(aag_statistics.get_num_ground_axiom_cache_misses_until_f_value().back(), 0);

    const auto& iw_statistics = iw.get_iw_statistics();

    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_generated_until_f_value().back(), 18);
    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_expanded_until_f_value().back(), 7);
    EXPECT_EQ(iw_statistics.get_effective_width(), 2);
}

}
