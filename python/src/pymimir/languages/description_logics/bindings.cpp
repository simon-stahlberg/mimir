#include "../init_declarations.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

using namespace mimir;

class IPyDLVisitor : public dl::Visitor
{
public:
    NB_TRAMPOLINE(dl::Visitor, 1);

    /* Trampoline (need one for each virtual function) */
    void visit(dl::ConceptBot constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
};

void bind_languages_description_logics(nb::module_& m) {}
