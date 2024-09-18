#ifndef MIMIR_PYTHON_TEST_FIXTURE_HPP
#define MIMIR_PYTHON_TEST_FIXTURE_HPP

#include "init_declarations.hpp"

#include <gtest/gtest.h>
#include <pybind11/embed.h>  // embed python interpreter to use from c++
#include <pybind11/pybind11.h>

// necessary to replicate the module here to use it in-place (verify against python/src/mimir/main.cpp)
PYBIND11_MODULE(_pymimir, m) { init_pymimir(m); }

PyMODINIT_FUNC PyInit__pymimir();

struct PymimirFixture : public ::testing::Test
{
    PymimirFixture()
    {
        PyImport_AppendInittab("_pymimir", &PyInit__pymimir);
        guard = std::make_unique<pybind11::scoped_interpreter>();
        m_pymimir = std::make_unique<pybind11::module>(pybind11::module::import("_pymimir"));
    }

    const auto& pymimir() const { return *m_pymimir; }

private:
    // unique ptr to ensure that the interpreter can be instantiated after PyImport_AppendInittab("pymimir",...)
    std::unique_ptr<pybind11::scoped_interpreter> guard = nullptr;
    std::unique_ptr<pybind11::module> m_pymimir = nullptr;
};

#endif  // MIMIR_PYTHON_TEST_FIXTURE_HPP
