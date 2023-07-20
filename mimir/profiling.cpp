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

#include "generators/successor_generator_factory.hpp"
#include "pddl/parsers.hpp"

#include <algorithm>
#include <chrono>
#include <deque>

// Older versions of LibC++ does not have filesystem (e.g., ubuntu 18.04), use the experimental version
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

std::vector<std::string> successor_generator_types() { return std::vector<std::string>({ "automatic", "lifted", "grounded" }); }

int main(int argc, char* argv[])
{
    std::string domain_path;
    std::string problem_path;
    std::string successor_generator_type;

    const auto successor_generators = successor_generator_types();

    if (argc != 4)
    {
        std::cout << "Invalid number of arguments" << std::endl;
        std::cout << "Profling <Domain> <Problem> <Successor Generator>" << std::endl;
        exit(1);
    }
    else
    {
        domain_path = argv[1];
        problem_path = argv[2];
        successor_generator_type = argv[3];
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

    if (std::count(successor_generators.begin(), successor_generators.end(), successor_generator_type) == 0)
    {
        std::cout << "Error: \"successor_generator\" does not exist" << std::endl;
        exit(4);
    }

    // parse PDDL files

    parsers::DomainParser domain_parser(domain_path);
    const auto domain = domain_parser.parse();
    std::cout << domain << std::endl;

    parsers::ProblemParser problem_parser(problem_path);
    const auto problem = problem_parser.parse(domain);
    std::cout << problem << std::endl;

    // find  plan

    auto generator = planners::SuccessorGeneratorType::AUTOMATIC;

    if (successor_generator_type == "lifted")
    {
        generator = planners::SuccessorGeneratorType::LIFTED;
    }
    else if (successor_generator_type == "grounded")
    {
        generator = planners::SuccessorGeneratorType::GROUNDED;
    }

    std::cout << "Using " << successor_generator_type << " successor generator" << std::endl << std::endl;

    // breadth-first search until a goal state is found

    struct Frame
    {
        formalism::State state;
        uint32_t depth;
    };

    std::cout << "Creating successor generator... ";
    const auto successor_generator = planners::create_sucessor_generator(problem, generator);
    std::cout << "Done" << std::endl;

    std::unordered_map<formalism::State, uint32_t> state_indices;
    std::deque<Frame> frame_list;
    std::deque<uint32_t> open_list;

    {  // Initialize data-structures
        // We want the index of the initial state to be 1 for convenience.
        frame_list.emplace_back(Frame { nullptr, 0 });

        // Add the initial state to the data-structures
        const uint32_t initial_index = static_cast<uint32_t>(frame_list.size());
        state_indices[problem->initial] = initial_index;
        frame_list.emplace_back(Frame { problem->initial, 0 });
        open_list.emplace_back(initial_index);
    }

    uint32_t expanded = 0;
    uint32_t generated = 0;
    uint32_t last_depth = 0;

    const auto time_start = std::chrono::high_resolution_clock::now();

    while (open_list.size() > 0)
    {
        const auto index = open_list.front();
        const auto frame = frame_list[index];
        open_list.pop_front();

        if (last_depth < frame.depth)
        {
            const auto time_depth = std::chrono::high_resolution_clock::now();
            const auto time_depth_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_depth - time_start).count();
            std::cout << "[g = " << last_depth << "] Expanded: " << expanded << "; Generated: " << generated << " [" << time_depth_ms << " ms]" << std::endl;
            last_depth = frame.depth;
        }

        if (formalism::literals_hold(problem->goal, frame.state))
        {
            std::cout << "Found goal state at depth " << frame.depth << std::endl;
            break;
        }

        ++expanded;

        const auto applicable_actions = successor_generator->get_applicable_actions(frame.state);

        for (const auto& action : applicable_actions)
        {
            const auto successor_state = formalism::apply(action, frame.state);
            auto& successor_index = state_indices[successor_state];

            // If successor_index is 0, then we haven't seen the state as it is reserved by the dummy frame that we added earlier.

            if (successor_index == 0)
            {
                ++generated;
                successor_index = static_cast<uint32_t>(frame_list.size());
                frame_list.emplace_back(Frame { successor_state, frame.depth + 1 });
                open_list.emplace_back(successor_index);
            }
        }
    }
}
