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


#include "grounded_successor_generator.hpp"
#include "lifted_successor_generator.hpp"
#include "successor_generator_factory.hpp"

namespace planners
{
    SuccessorGenerator create_sucessor_generator(const formalism::ProblemDescription& problem, SuccessorGeneratorType type)
    {
        switch (type)
        {
            case SuccessorGeneratorType::AUTOMATIC:
            {
                const auto lifted_successor_generator = std::make_shared<LiftedSuccessorGenerator>(problem->domain, problem);

                formalism::ActionList actions;
                if (lifted_successor_generator->get_actions(60, actions))
                {
                    // The GroundedSuccessorGenerator does not play well with an extreme number of actions.

                    if (actions.size() < 100'000)
                    {
                        return std::make_shared<GroundedSuccessorGenerator>(problem, actions);
                    }
                }

                return lifted_successor_generator;
            }

            case SuccessorGeneratorType::LIFTED:
            {
                return std::make_shared<LiftedSuccessorGenerator>(problem->domain, problem);
            }

            case SuccessorGeneratorType::GROUNDED:
            {
                // The lifted successor generator can overapproximate all actions applicable in reachable states by only taking static atoms into
                // considerations. The grounded successor generator is a decision tree structure over these actions.

                LiftedSuccessorGenerator lifted_successor_generator(problem->domain, problem);
                return std::make_shared<GroundedSuccessorGenerator>(problem, lifted_successor_generator.get_actions());
            }

            default:
            {
                throw std::runtime_error("successor generator type is not yet implemented");
            }
        }
    }
}  // namespace planners
