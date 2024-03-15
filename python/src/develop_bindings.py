#!/usr/bin/env python3

import sys

from pathlib import Path

ROOT_DIR = (Path.cwd().parent.parent).absolute()
LIB_PATH = ROOT_DIR / "build" / "python" / "src"

sys.path.append(str(LIB_PATH))

from mimir.formalism import PDDLParser
from mimir.search import *


if __name__ == "__main__":
    parser = PDDLParser(str(ROOT_DIR / "data" / "gripper" / "domain.pddl"), str(ROOT_DIR / "data" / "gripper" / "problem.pddl"))
    print(parser.get_domain().get_actions())
    print(parser.get_problem().get_objects())
