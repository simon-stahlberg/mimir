import pymimir.advanced.formalism as formalism

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_pddl_parser():
    """ Test parsing and translation of a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = formalism.Parser(domain_filepath)

    domain = parser.get_domain()

    problem = parser.parse_problem(problem_filepath)

    assert(domain == problem.get_domain())

    translator = formalism.Translator(domain)
    translated_domain = translator.get_translated_domain()

    translated_problem = translator.translate(problem)

    assert(translated_domain == translated_problem.get_domain())
