#if !defined(FORMALISM_PRINT_FUNCTIONS_HPP_)
#define FORMALISM_PRINT_FUNCTIONS_HPP_

#include "atom.hpp"

#include <iostream>

namespace formalism
{
    void print_atom(const formalism::Atom& atom)
    {
        std::cout << atom->predicate->name << "(";

        for (uint32_t argument_index = 0; argument_index < atom->arguments.size(); ++argument_index)
        {
            const auto object = atom->arguments.at(argument_index);
            std::cout << object->name;

            if ((argument_index + 1) < atom->arguments.size())
            {
                std::cout << ", ";
            }
        }

        std::cout << ")" << std::endl;
    }

    void print_atoms(const formalism::AtomList& atoms)
    {
        for (const auto& atom : atoms)
        {
            print_atom(atom);
        }
    }
}  // namespace formalism

#endif  // FORMALISM_PRINT_FUNCTIONS_HPP_
