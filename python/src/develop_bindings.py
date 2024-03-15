#!/usr/bin/env python3

import sys

from pathlib import Path

ROOT_DIR = (Path.cwd().parent.parent).absolute()
LIB_PATH = ROOT_DIR / "build" / "python" / "src"

sys.path.append(str(LIB_PATH))

from mimir.formalism import PDDLParser
from mimir.search import SSG, LiftedAAG, MinimalEventHandler, BrFsAlgorithm


if __name__ == "__main__":
    parser = PDDLParser(str(ROOT_DIR / "data" / "gripper" / "domain.pddl"), str(ROOT_DIR / "data" / "gripper" / "problem.pddl"))
    print(str(parser.get_domain()))
    print(str(parser.get_problem()))

    ssg = SSG(parser.get_problem())
    aag = LiftedAAG(parser.get_problem(), parser.get_factories())
    event_handler = MinimalEventHandler()
    brfs = BrFsAlgorithm(parser.get_problem(), parser.get_factories(), ssg, aag, event_handler)
    brfs.find_solution()
