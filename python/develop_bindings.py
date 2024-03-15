#!/usr/bin/env python3

import sys

from pathlib import Path

ROOT_DIR = (Path.cwd().parent).absolute()
LIB_PATH = ROOT_DIR / "build" / "python"

sys.path.append(str(LIB_PATH))

from src.mimir.formalism import PDDLParser
from src.mimir.search import *


if __name__ == "__main__":

    parser = PDDLParser(str(ROOT_DIR / "data" / "gripper" / "domain.pddl"), str(ROOT_DIR / "data" / "gripper" / "problem.pddl"))