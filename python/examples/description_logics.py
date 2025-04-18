import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets
import pymimir.advanced.languages.description_logics as description_logics

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()

domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

# Create a state space (could also use knowledge base)
state_space, certificates = datasets.StateSpace.create(search.SearchContext.create(domain_filepath, problem_filepath))
domain = state_space.get_search_context().get_problem().get_domain()

# Create a CNFGrammar for the domain.
grammar = description_logics.CNFGrammar.create(description_logics.GrammarSpecificationEnum.FRANCES_ET_AL_AAAI2021, domain)
print(grammar)