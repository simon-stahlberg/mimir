/*
 * Copyright (C) 2023 Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../include/mimir/datastructures/robin_map.hpp"
#include "../include/mimir/generators/complete_state_space.hpp"
#include "../include/mimir/generators/grounded_successor_generator.hpp"
#include "../include/mimir/generators/successor_generator_factory.hpp"
#include "../include/mimir/pddl/parsers.hpp"
#include "../include/mimir/search/breadth_first_search.hpp"
#include "../include/mimir/search/eager_astar_search.hpp"
#include "../include/mimir/search/heuristics/h1_heuristic.hpp"
#include "../include/mimir/search/heuristics/h2_heuristic.hpp"
#include "../include/mimir/search/openlists/priority_queue_open_list.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <numeric>
#include <queue>
#include <vector>

// Older versions of LibC++ does not have filesystem (e.g., ubuntu 18.04), use the experimental version
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

// Global variables to keep track of memory allocation and deallocation
std::size_t total_memory_allocated = 0;
std::size_t total_memory_deallocated = 0;
std::size_t peak_memory_usage = 0;

// Override the new operator
void* operator new(std::size_t size)
{
    const auto pointer = static_cast<int32_t*>(std::malloc(size + sizeof(int32_t)));

    if (pointer)
    {
        pointer[0] = size;
        total_memory_allocated += size;
        peak_memory_usage = std::max(peak_memory_usage, total_memory_allocated - total_memory_deallocated);
        return pointer + 1;
    }
    else
    {
        throw std::bad_alloc();
    }
}

// Override the delete operator
void operator delete(void* ptr) noexcept
{
    if (ptr)
    {
        const auto original_pointer = static_cast<int32_t*>(ptr) - 1;
        const auto size = *original_pointer;
        total_memory_deallocated += static_cast<std::size_t>(size);
        std::free(original_pointer);
    }
}

// Override the new[] operator
void* operator new[](std::size_t size) { return operator new(size); }

// Override the delete[] operator
void operator delete[](void* ptr) noexcept { operator delete(ptr); }

std::vector<std::string> successor_generator_types() { return std::vector<std::string>({ "automatic", "lifted", "grounded" }); }

void bfs(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator)
{
    mimir::planners::Search search = mimir::planners::create_breadth_first_search(problem, successor_generator);
    const auto time_start = std::chrono::high_resolution_clock::now();

    search->register_handler(
        [&time_start, &search]()
        {
            const auto time_now = std::chrono::high_resolution_clock::now();
            const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_start).count();
            const auto statistics = search->get_statistics();
            const auto expanded = std::get<int32_t>(statistics.at("expanded"));
            const auto generated = std::get<int32_t>(statistics.at("generated"));
            const auto depth = std::get<int32_t>(statistics.at("max_depth"));
            std::cout << "[depth = " << depth << "] Expanded: " << expanded << "; Generated: " << generated << " [" << time_delta << " ms; ";
            std::cout << ((total_memory_allocated - total_memory_deallocated) / 1000) << " KB; " << (peak_memory_usage / 1000) << " KB peak]" << std::endl;
        });

    mimir::formalism::ActionList plan;
    const auto result = search->plan(plan);

    std::cout << std::endl;

    switch (result)
    {
        case mimir::planners::SearchResult::SOLVED:
            std::cout << "Found a plan of length " << plan.size() << ":" << std::endl;
            for (const auto& action : plan)
            {
                std::cout << action << std::endl;
            }
            break;

        case mimir::planners::SearchResult::UNSOLVABLE:
            std::cout << "Problem is provably unsolvable" << std::endl;
            break;

        case mimir::planners::SearchResult::ABORTED:
            std::cout << "Search was aborted" << std::endl;
            break;

        default:
            throw std::runtime_error("not implemented");
    }
}

void state_space(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator)
{
    const auto state_space = mimir::planners::create_complete_state_space(problem, successor_generator, 100'000);

    if (state_space)
    {
        const auto num_objects = problem->num_objects();
        const auto num_states = state_space->num_states();
        const auto num_dead_end_states = state_space->num_dead_end_states();
        const auto num_goal_states = state_space->num_goal_states();

        std::cout << "# Objects: " << num_objects << std::endl;
        std::cout << "# States: " << num_states << std::endl;
        std::cout << "# Dead End States: " << num_dead_end_states << std::endl;
        std::cout << "# Goal States: " << num_goal_states << std::endl;
    }
    else
    {
        std::cout << "Problem too large to expand" << std::endl;
    }
}

void astar(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator)
{
    const auto time_start = std::chrono::high_resolution_clock::now();
    const auto heuristic = mimir::planners::create_h1_heuristic(problem, successor_generator);
    const auto open_list = mimir::planners::create_priority_queue_open_list();
    auto search = mimir::planners::create_eager_astar(problem, successor_generator, heuristic, open_list);

    search->register_handler(
        [&time_start, &search]()
        {
            const auto time_now = std::chrono::high_resolution_clock::now();
            const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_start).count();
            const auto statistics = search->get_statistics();
            const auto expanded = std::get<int32_t>(statistics.at("expanded"));
            const auto generated = std::get<int32_t>(statistics.at("generated"));
            const auto evaluated = std::get<int32_t>(statistics.at("evaluated"));
            // const auto depth = std::get<int32_t>(statistics.at("max_depth"));
            // const auto g_value = std::get<double>(statistics.at("max_g_value"));
            const auto f_value = std::get<double>(statistics.at("max_f_value"));
            std::cout << "[f = " << f_value << "] Expanded: " << expanded << "; Generated: " << generated << "; Evaluated: " << evaluated << " [" << time_delta
                      << " ms; ";
            std::cout << ((total_memory_allocated - total_memory_deallocated) / 1000) << " KB; " << (peak_memory_usage / 1000) << " KB peak]" << std::endl;
        });

    mimir::formalism::ActionList plan;
    const auto result = search->plan(plan);

    std::cout << std::endl;

    switch (result)
    {
        case mimir::planners::SearchResult::SOLVED:
            std::cout << "Found a plan of length " << plan.size() << ":" << std::endl;
            for (const auto& action : plan)
            {
                std::cout << action << std::endl;
            }
            break;

        case mimir::planners::SearchResult::UNSOLVABLE:
            std::cout << "Problem is provably unsolvable" << std::endl;
            break;

        case mimir::planners::SearchResult::ABORTED:
            std::cout << "Search was aborted" << std::endl;
            break;

        default:
            throw std::runtime_error("not implemented");
    }
}

void dijkstra(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator)
{
    // dijkstra's until a goal state is found

    struct Frame
    {
        mimir::formalism::State state;
        double f;
    };

    mimir::tsl::robin_map<mimir::formalism::State, uint32_t> state_indices;
    std::deque<Frame> frame_list;
    const auto comparator = [](const std::pair<double, int>& lhs, const std::pair<double, int>& rhs) { return lhs.first > rhs.first; };
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, decltype(comparator)> priority_queue(comparator);

    {  // Initialize data-structures
        // We want the index of the initial state to be 1 for convenience.
        frame_list.emplace_back(Frame { nullptr, 0 });

        // Add the initial state to the data-structures
        const uint32_t initial_index = static_cast<uint32_t>(frame_list.size());
        const auto initial_state = mimir::formalism::create_state(problem->initial, problem);
        state_indices[initial_state] = initial_index;
        frame_list.emplace_back(Frame { initial_state, 0 });
        priority_queue.emplace(0.0, initial_index);
    }

    uint32_t expanded = 0;
    uint32_t generated = 0;
    double last_f = std::numeric_limits<double>::min();

    const auto time_start = std::chrono::high_resolution_clock::now();

    while (priority_queue.size() > 0)
    {
        const auto index = priority_queue.top().second;
        const auto& frame = frame_list[index];
        priority_queue.pop();

        if (last_f < frame.f)
        {
            last_f = frame.f;
            const auto time_f = std::chrono::high_resolution_clock::now();
            const auto time_f_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_f - time_start).count();
            std::cout << "[f = " << last_f << "] Expanded: " << expanded << "; Generated: " << generated << " [" << time_f_ms << " ms]" << std::endl;
        }

        if (mimir::formalism::literals_hold(problem->goal, frame.state))
        {
            std::cout << "Found goal state at f-value " << frame.f << std::endl;
            break;
        }

        ++expanded;

        const auto applicable_actions = successor_generator->get_applicable_actions(frame.state);

        for (const auto& action : applicable_actions)
        {
            const auto successor_state = mimir::formalism::apply(action, frame.state);
            auto& successor_index = state_indices[successor_state];

            // If successor_index is 0, then we haven't seen the state as it is reserved by the dummy frame that we added earlier.

            if (successor_index == 0)
            {
                ++generated;
                const auto successor_f_value = frame.f + action->cost;
                successor_index = static_cast<uint32_t>(frame_list.size());
                frame_list.emplace_back(Frame { successor_state, successor_f_value });
                priority_queue.emplace(successor_f_value, successor_index);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    std::string domain_path;
    std::string problem_path;
    std::string generator_name;
    std::string search_name;

    const auto successor_generators = successor_generator_types();

    if (argc != 5)
    {
        std::cout << "Invalid number of arguments" << std::endl;
        std::cout << "Profling <Domain> <Problem> <Successor Generator> <dijkstras|bfs|astar|statespace>" << std::endl;
        exit(1);
    }
    else
    {
        domain_path = argv[1];
        problem_path = argv[2];
        generator_name = argv[3];
        search_name = argv[4];
    }

    if (!fs::exists(domain_path))
    {
        std::cout << "Error: \"domain\" does not exist" << std::endl;
        exit(2);
    }

    if (!fs::exists(problem_path))
    {
        std::cout << "Error: \"problem\" does not exist" << std::endl;
        exit(3);
    }

    if (std::count(successor_generators.begin(), successor_generators.end(), generator_name) == 0)
    {
        std::cout << "Error: \"successor_generator\" does not exist" << std::endl;
        exit(4);
    }

    // parse PDDL files

    mimir::parsers::DomainParser domain_parser(domain_path);
    const auto domain = domain_parser.parse();
    std::cout << domain << std::endl;

    mimir::parsers::ProblemParser problem_parser(problem_path);
    const auto problem = problem_parser.parse(domain);
    std::cout << problem << std::endl;

    // find  plan

    auto generator = mimir::planners::SuccessorGeneratorType::AUTOMATIC;

    if (generator_name == "lifted")
    {
        generator = mimir::planners::SuccessorGeneratorType::LIFTED;
    }
    else if (generator_name == "grounded")
    {
        generator = mimir::planners::SuccessorGeneratorType::GROUNDED;
    }

    std::cout << "Using " << generator_name << " successor generator" << std::endl << std::endl;

    std::cout << "Creating successor generator... ";
    const auto successor_generator = mimir::planners::create_sucessor_generator(problem, generator);
    std::cout << "Done" << std::endl;

    if (generator == mimir::planners::SuccessorGeneratorType::GROUNDED)
    {
        const auto grounded_successor_generator = std::static_pointer_cast<mimir::planners::GroundedSuccessorGenerator>(successor_generator);
        std::cout << "Number of ground actions: " << grounded_successor_generator->get_actions().size() << std::endl;
        std::cout << "Number of atoms: " << problem->get_encountered_atoms().size() << std::endl;
        std::cout << "Number of static atoms: " << problem->get_static_atoms().size() << std::endl;
    }

    std::cout << std::endl;

    if (search_name == "bfs")
    {
        bfs(problem, successor_generator);
    }
    else if (search_name == "dijkstras")
    {
        dijkstra(problem, successor_generator);
    }
    else if (search_name == "astar")
    {
        astar(problem, successor_generator);
    }
    else if (search_name == "statespace")
    {
        state_space(problem, successor_generator);
    }
    else
    {
        std::cout << "Error: \"" << search_name << "\" is an invalid search type" << std::endl;
    }
}
