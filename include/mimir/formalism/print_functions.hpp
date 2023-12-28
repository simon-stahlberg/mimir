#ifndef MIMIR_FORMALISM_PRINT_FUNCTIONS_HPP_
#define MIMIR_FORMALISM_PRINT_FUNCTIONS_HPP_

#include "atom.hpp"

#include <iostream>

namespace mimir::formalism
{
    void print_atom(const mimir::formalism::Atom& atom)
    {
        std::cout << atom.get_predicate()->name << "(";

        for (uint32_t argument_index = 0; argument_index < atom.get_terms().size(); ++argument_index)
        {
            const auto object = atom.get_terms().at(argument_index);
            std::cout << object->name;

            if ((argument_index + 1) < atom.get_terms().size())
            {
                std::cout << ", ";
            }
        }

        std::cout << ")" << std::endl;
    }

    void print_atoms(const mimir::formalism::AtomList& atoms)
    {
        for (const auto& atom : atoms)
        {
            print_atom(atom);
        }
    }
}  // namespace formalism

#endif  // MIMIR_FORMALISM_PRINT_FUNCTIONS_HPP_
