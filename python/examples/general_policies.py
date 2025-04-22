""" This example illustrates the interface of the description logics component.
"""

import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets
import pymimir.advanced.languages.description_logics as description_logics
import pymimir.advanced.languages.general_policies as general_policies

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()


def main():
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")

    # Create some states
    search_context_options = search.SearchContextOptions()
    search_context_options.mode = search.SearchMode.GROUNDED
    generalized_search_context = search.GeneralizedSearchContext.create(domain_filepath, [problem_filepath], search_context_options)

    state_space_options = datasets.StateSpaceOptions()
    state_space_options.symmetry_pruning = True 
    generalized_state_space_options = datasets.GeneralizedStateSpaceOptions()
    knowledge_base_options = datasets.KnowledgeBaseOptions()
    knowledge_base_options.state_space_options = state_space_options
    knowledge_base_options.generalized_state_space_options = generalized_state_space_options
    knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

    repositories = general_policies.Repositories()
    dl_repositories = description_logics.Repositories()
    denotation_repositories = description_logics.DenotationRepositories()

    # Create a policy from description.
    general_policy = repositories.get_or_create_general_policy(
        "[boolean_features]" \
        "[numerical_features]" \
        "[policy_rules]", 
        knowledge_base.get_domain(), dl_repositories)
    print(general_policy)
    assert(general_policy.is_terminating(repositories))
    assert(general_policy.solves(knowledge_base.get_generalized_state_space(), denotation_repositories) == general_policies.SolvabilityStatus.UNSOLVABLE)

    # Create a policy off-the-shelf.
    general_policy = general_policies.GeneralPolicyFactory.get_or_create_general_policy_gripper(knowledge_base.get_domain(), repositories, dl_repositories)
    print(general_policy)
    assert(general_policy.is_terminating(repositories))
    assert(general_policy.solves(knowledge_base.get_generalized_state_space(), denotation_repositories) == general_policies.SolvabilityStatus.SOLVED)

if __name__ == "__main__":
    main()
