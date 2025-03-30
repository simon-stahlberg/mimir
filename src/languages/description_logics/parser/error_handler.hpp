/*
 * Copyright (C) 2023 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_PARSER_ERROR_HANDLER_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_PARSER_ERROR_HANDLER_HPP_

#include "../../../common/x3/config.hpp"

#include <string>
#include <unordered_map>

namespace mimir::languages::dl
{
namespace x3 = boost::spirit::x3;

////////////////////////////////////////////////////////////////////////////
// Our error handler
////////////////////////////////////////////////////////////////////////////

struct error_handler_base
{
    std::unordered_map<std::string, std::string> id_map;

    error_handler_base() {}

    template<typename Iterator, typename Exception, typename Context>
    x3::error_handler_result on_error(Iterator& /*first*/,
                                      Iterator const& /*last*/
                                      ,
                                      Exception const& x,
                                      Context const& context)
    {
        {
            std::string which = x.which();
            auto iter = id_map.find(which);
            if (iter != id_map.end())
            {
                which = iter->second;
            }

            std::string message = "Error! Expecting: " + which + " here:";
            auto& error_handler = x3::get<mimir::x3::error_handler_tag>(context).get();
            error_handler(x.where(), message);

            return x3::error_handler_result::fail;
        }
    }
};
}

#endif
