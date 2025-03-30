/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/common/filesystem.hpp"

namespace mimir
{
void write_to_file(const fs::path& filePath, const std::string& content)
{
    // Ensure that the directory path exists; create it if it does not.
    if (filePath.has_parent_path())
    {
        fs::create_directories(filePath.parent_path());
    }

    // Attempt to open the file with ofstream in write mode.
    std::ofstream fileStream(filePath);

    // Check if the file stream is open and ready for writing.
    if (!fileStream.is_open())
    {
        std::cerr << "Failed to open the file: " << filePath << std::endl;
        return;
    }

    // Write the content to the file.
    fileStream << content;

    // Optionally, check for write errors.
    if (!fileStream)
    {
        std::cerr << "Failed to write to the file: " << filePath << std::endl;
    }

    // Close the file stream automatically when the function exits and fileStream goes out of scope.
}
}
