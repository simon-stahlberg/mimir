# Import all classes for better IDE support


from pymimir.pymimir.advanced.graphs import (
    # Properties
    IProperty,
    PyProperty,
    PropertyValue,

    # Vertices
    PyVertex,

    # Edges
    PyEdge,

    # Nauty
    NautySparseGraph,

    # StaticPyGraph
    StaticPyGraph,
    ImmutableStaticPyGraph,
    ForwardStaticPyGraph,
    BidirectionalStaticPyGraph,
    DynamicPyGraph,
)

# Coloring
from pymimir.pymimir.advanced.graphs import (
    compute_color_refinement_certificate,
    compute_2fwl_certificate,
    compute_3fwl_certificate,
    compute_4fwl_certificate,
    CertificateColorRefinement,
    KFWLIsomorphismTypeCompressionFunction,
    Certficate2FWL,
    Certficate3FWL,
    Certficate4FWL,
)

# graph properties
from pymimir.pymimir.advanced.graphs import (
    is_undirected,
    is_loopless,
    is_acyclic,
    is_multi,
)
