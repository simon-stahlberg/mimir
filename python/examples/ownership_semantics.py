#! /usr/bin/env python

from pymimir import PDDLParser, LiftedAAG, SuccessorStateGenerator

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()

if __name__ == "__main__":
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")

    # Ownership semantics rule 1: do not unintentionally free memory of PDDLParser
    # You should assign it to a variable and do not reassign it.
    pddl_parser = PDDLParser(domain_filepath, problem_filepath)
    # Ownership semantics rule 2: do not unintentionally free memory of ApplicableActionGenerator (AAG)
    # You should assign it to a variable and do not reassign it.
    lifted_aag = LiftedAAG(pddl_parser.get_problem(), pddl_parser.get_factories())
    # Ownership semantics rule 3: do not unintentionally free memory of SuccessorStateGenerator (SuccessorStateGenerator)
    # You should assign it to a variable and do not reassign it.
    ssg = SuccessorStateGenerator(lifted_aag)

    # Do some work and access data from PDDLParser, AAG, and SuccessorStateGenerator
    initial_state = ssg.get_or_create_initial_state()
    actions = lifted_aag.compute_applicable_actions(initial_state)

    print(initial_state.to_string(pddl_parser.get_problem(), pddl_parser.get_factories()))

    for action in actions:
        print(action.to_string(pddl_parser.get_factories()))
