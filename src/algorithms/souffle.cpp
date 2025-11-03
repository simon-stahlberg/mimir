/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "mimir/algorithms/souffle.hpp"

#include "MainDriver.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mimir::datalog
{

/// A virtual file system containing a single file.
struct SingleFileFS : public souffle::FileSystem
{
    explicit SingleFileFS(std::filesystem::path FilePath, std::string FileContent);

    bool exists(const std::filesystem::path& Path) override;

    std::filesystem::path canonical(const std::filesystem::path& Path, std::error_code& EC) override;

    std::string readFile(const std::filesystem::path& Path, std::error_code& EC) override;

private:
    const std::filesystem::path FilePath;
    const std::string FileContent;
};

SingleFileFS::SingleFileFS(std::filesystem::path FilePath, std::string FileContent) : FilePath(FilePath), FileContent(FileContent) {}

bool SingleFileFS::exists(const std::filesystem::path& Path) { return (Path == FilePath); }

std::filesystem::path SingleFileFS::canonical(const std::filesystem::path& Path, std::error_code& EC)
{
    if (exists(Path))
    {
        EC = std::error_code {};
        return Path;
    }
    else
    {
        EC = std::make_error_code(std::errc::no_such_file_or_directory);
        return "";
    }
}

std::string SingleFileFS::readFile(const std::filesystem::path& Path, std::error_code& EC)
{
    if (canonical(Path, EC) == Path && EC == std::error_code {})
    {
        return FileContent;
    }
    else
    {
        return std::string {};
    }
}

int solve(const std::string& program, std::shared_ptr<souffle::WriteStreamFactory> write_stream_factory)
{
    auto InputDl = std::make_shared<SingleFileFS>("input.dl", program);

    try
    {
        souffle::Global glb;
        glb.config().set("jobs", "1");
        glb.config().set("", "input.dl");
        glb.config().set("output-dir", ".");

        souffle::IOSystem::getInstance().registerWriteStreamFactory(std::move(write_stream_factory));

        souffle::ErrorReport errReport;
        souffle::DebugReport dbgReport(glb);

        auto VFS = std::make_shared<souffle::OverlayFileSystem>(InputDl);
        auto astTranslationUnit = souffle::ParserDriver::parseTranslationUnitFromFS(glb, "input.dl", errReport, dbgReport, VFS);

        auto pipeline = astTransformationPipeline(glb);
        pipeline->apply(*astTranslationUnit);

        auto unitTranslator = getUnitTranslator(glb);
        auto ramTranslationUnit = unitTranslator->translateUnit(*astTranslationUnit);

        auto ramTransform = ramTransformerSequence(glb);
        ramTransform->apply(*ramTranslationUnit);

        const bool success = interpretTranslationUnit(glb, *ramTranslationUnit);
        if (!success)
        {
            std::exit(EXIT_FAILURE);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return 0;
}

}
