from . import advanced as advanced

from .wrapper_formalism import (
    Action,
    Atom,
    ConditionalEffect,
    ConjunctiveCondition,
    Domain,
    Effect,
    GroundAction,
    GroundAtom,
    GroundConditionalEffect,
    GroundConjunctiveCondition,
    GroundEffect,
    GroundLiteral,
    Literal,
    Object,
    Predicate,
    Problem,
    State,
    Variable,
)

from .wrapper_heuristics import (
    AddHeuristic,
    BlindHeuristic,
    FFHeuristic,
    H2Heuristic,
    Heuristic,
    MaxHeuristic,
    PerfectHeuristic,
    SetAddHeuristic,
)

from .wrapper_search import (
    SearchResult,
)

from .wrapper_search_astar import (
    astar_eager,
    astar_lazy,
)

from .wrapper_search_brfs import (
    brfs,
)

from .wrapper_search_gbfs import (
    gbfs_eager,
    gbfs_lazy,
)

from .wrapper_search_width import (
    iw,
)

from .wrapper_datasets import (
    StateSpaceSampler,
    StateLabel,
)

from pathlib import Path


def _package_root() -> Path:
    return Path(__file__).resolve().parent


def get_include_dir() -> str:
    return str(_package_root() / "include")


def get_include() -> str:
    return get_include_dir()


def get_cmake_dir() -> str:
    pkg_root = _package_root()
    for libdir in ("lib", "lib64"):
        cmake_dir = pkg_root / libdir / "cmake" / "mimir"
        if cmake_dir.is_dir():
            return str(cmake_dir)
    return str(pkg_root / "lib" / "cmake" / "mimir")
