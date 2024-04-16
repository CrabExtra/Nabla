// Copyright (C) 2018-2022 - DevSH Graphics Programming Sp. z O.O.
// This file is part of the "Nabla Engine".
// For conditions of distribution and use, see copyright notice in nabla.h
#include "nbl/asset/utils/IShaderCompiler.h"
#include "nbl/asset/utils/shadercUtils.h"
#include "nbl/asset/utils/CGLSLVirtualTexturingBuiltinIncludeGenerator.h"
#include "nbl/asset/utils/shaderCompiler_serialization.h"
#include "nbl/core/xxHash256.h"

#include <sstream>
#include <regex>
#include <iterator>

using namespace nbl;
using namespace nbl::asset;

IShaderCompiler::IShaderCompiler(core::smart_refctd_ptr<system::ISystem>&& system)
    : m_system(std::move(system))
{
    m_defaultIncludeFinder = core::make_smart_refctd_ptr<CIncludeFinder>(core::smart_refctd_ptr(m_system));
    m_defaultIncludeFinder->addGenerator(core::make_smart_refctd_ptr<asset::CGLSLVirtualTexturingBuiltinIncludeGenerator>());
    m_defaultIncludeFinder->getIncludeStandard("", "nbl/builtin/glsl/utils/common.glsl");
}

std::string IShaderCompiler::preprocessShader(
    system::IFile* sourcefile,
    IShader::E_SHADER_STAGE stage,
    const SPreprocessorOptions& preprocessOptions,
    std::vector<CCache::SEntry::SPreprocessingDependency>* dependencies) const
{
    std::string code(sourcefile->getSize(), '\0');

    system::IFile::success_t success;
    sourcefile->read(success, code.data(), 0, sourcefile->getSize());
    if (!success)
        return nullptr;

    return preprocessShader(std::move(code), stage, preprocessOptions, dependencies);
}

auto IShaderCompiler::IIncludeGenerator::getInclude(const std::string& includeName) const -> IIncludeLoader::found_t
{
    core::vector<std::pair<std::regex, HandleFunc_t>> builtinNames = getBuiltinNamesToFunctionMapping();
    for (const auto& pattern : builtinNames)
    if (std::regex_match(includeName,pattern.first))
    {
        if (auto contents=pattern.second(includeName); !contents.empty())
        {
            // Welcome, you've came to a very disused piece of code, please check the first parameter (path) makes sense!
            _NBL_DEBUG_BREAK_IF(true);
            return {includeName,contents};
        }
    }

    return {};
}

core::vector<std::string> IShaderCompiler::IIncludeGenerator::parseArgumentsFromPath(const std::string& _path)
{
    core::vector<std::string> args;

    std::stringstream ss{ _path };
    std::string arg;
    while (std::getline(ss, arg, '/'))
        args.push_back(std::move(arg));

    return args;
}

IShaderCompiler::CFileSystemIncludeLoader::CFileSystemIncludeLoader(core::smart_refctd_ptr<system::ISystem>&& system) : m_system(std::move(system))
{}

auto IShaderCompiler::CFileSystemIncludeLoader::getInclude(const system::path& searchPath, const std::string& includeName) const -> found_t
{
    system::path path = searchPath / includeName;
    if (std::filesystem::exists(path))
        path = std::filesystem::canonical(path);

    core::smart_refctd_ptr<system::IFile> f;
    {
        system::ISystem::future_t<core::smart_refctd_ptr<system::IFile>> future;
        m_system->createFile(future, path.c_str(), system::IFile::ECF_READ);
        if (!future.wait())
            return {};
        future.acquire().move_into(f);
    }
    if (!f)
        return {};
    const size_t size = f->getSize();

    std::string contents(size, '\0');
    system::IFile::success_t succ;
    f->read(succ, contents.data(), 0, size);
    const bool success = bool(succ);
    assert(success);

    return {f->getFileName(),std::move(contents)};
}

IShaderCompiler::CIncludeFinder::CIncludeFinder(core::smart_refctd_ptr<system::ISystem>&& system) 
    : m_defaultFileSystemLoader(core::make_smart_refctd_ptr<CFileSystemIncludeLoader>(std::move(system)))
{
    addSearchPath("", m_defaultFileSystemLoader);
}

// ! includes within <>
// @param requestingSourceDir: the directory where the incude was requested
// @param includeName: the string within <> of the include preprocessing directive
auto IShaderCompiler::CIncludeFinder::getIncludeStandard(const system::path& requestingSourceDir, const std::string& includeName) const -> IIncludeLoader::found_t
{
    IShaderCompiler::IIncludeLoader::found_t retVal;
    if (auto contents = tryIncludeGenerators(includeName)) 
        retVal = std::move(contents);
    else if (auto contents = trySearchPaths(includeName)) 
        retVal = std::move(contents);
    else retVal = std::move(m_defaultFileSystemLoader->getInclude(requestingSourceDir.string(),includeName));

    retVal.hash = nbl::core::XXHash_256((uint8_t*)(retVal.contents.data()), retVal.contents.size() * (sizeof(char) / sizeof(uint8_t)));
    return retVal;
}

// ! includes within ""
// @param requestingSourceDir: the directory where the incude was requested
// @param includeName: the string within "" of the include preprocessing directive
auto IShaderCompiler::CIncludeFinder::getIncludeRelative(const system::path& requestingSourceDir, const std::string& includeName) const -> IIncludeLoader::found_t
{
    IShaderCompiler::IIncludeLoader::found_t retVal;
    if (auto contents = m_defaultFileSystemLoader->getInclude(requestingSourceDir.string(),includeName))
        retVal = std::move(contents);
    else retVal = std::move(trySearchPaths(includeName));
    retVal.hash = nbl::core::XXHash_256((uint8_t*)(retVal.contents.data()), retVal.contents.size() * (sizeof(char) / sizeof(uint8_t)));
    return retVal;
}

void IShaderCompiler::CIncludeFinder::addSearchPath(const std::string& searchPath, const core::smart_refctd_ptr<IIncludeLoader>& loader)
{
    if (!loader)
        return;
    m_loaders.push_back(LoaderSearchPath{ loader, searchPath });
}

void IShaderCompiler::CIncludeFinder::addGenerator(const core::smart_refctd_ptr<IIncludeGenerator>& generatorToAdd)
{
    if (!generatorToAdd)
        return;

    // this will find the place of first generator with prefix <= generatorToAdd or end
    auto found = std::lower_bound(m_generators.begin(), m_generators.end(), generatorToAdd->getPrefix(),
        [](const core::smart_refctd_ptr<IIncludeGenerator>& generator, const std::string_view& value)
        {
            auto element = generator->getPrefix();
            return element.compare(value) > 0; // first to return false is lower_bound -> first element that is <= value
        });

    m_generators.insert(found, generatorToAdd);
}

auto IShaderCompiler::CIncludeFinder::trySearchPaths(const std::string& includeName) const -> IIncludeLoader::found_t
{
    for (const auto& itr : m_loaders)
    if (auto contents = itr.loader->getInclude(itr.searchPath,includeName))
        return contents;
    return {};
}

auto IShaderCompiler::CIncludeFinder::tryIncludeGenerators(const std::string& includeName) const -> IIncludeLoader::found_t
{
    // Need custom function because std::filesystem doesn't consider the parameters we use after the extension like CustomShader.hlsl/512/64
    auto removeExtension = [](const std::string& str)
    {
        return str.substr(0, str.find_last_of('.'));
    };

    auto standardizePrefix = [](const std::string_view& prefix) -> std::string
    {
        std::string ret(prefix);
        // Remove Trailing '/' if any, to compare to filesystem paths
        if (*ret.rbegin() == '/' && ret.size() > 1u)
            ret.resize(ret.size() - 1u);
        return ret;
    };

    auto extension_removed_path = system::path(removeExtension(includeName));
    system::path path = extension_removed_path.parent_path();

    // Try Generators with Matching Prefixes:
    // Uses a "Path Peeling" method which goes one level up the directory tree until it finds a suitable generator
    auto end = m_generators.begin();
    while (!path.empty() && path.root_name().empty() && end != m_generators.end())
    {
        auto begin = std::lower_bound(end, m_generators.end(), path.string(),
            [&standardizePrefix](const core::smart_refctd_ptr<IIncludeGenerator>& generator, const std::string& value)
            {
                const auto element = standardizePrefix(generator->getPrefix());
                return element.compare(value) > 0; // first to return false is lower_bound -> first element that is <= value
            });

        // search from new beginning to real end
        end = std::upper_bound(begin, m_generators.end(), path.string(),
            [&standardizePrefix](const std::string& value, const core::smart_refctd_ptr<IIncludeGenerator>& generator)
            {
                const auto element = standardizePrefix(generator->getPrefix());
                return value.compare(element) > 0; // first to return true is upper_bound -> first element that is < value
            });

        for (auto generatorIt = begin; generatorIt != end; generatorIt++)
        {
            if (auto contents = (*generatorIt)->getInclude(includeName))
                return contents;
        }

        path = path.parent_path();
    }

    return {};
}

std::vector<uint8_t> IShaderCompiler::CCache::serialize()
{
    std::vector<uint8_t> shadersBuffer;
    // Push back the entries that were already serialized straight into the buffer
    if (storageBuffer) {
        shadersBuffer.resize(storageBuffer->getSize() - containerJsonSize - CONTAINER_JSON_SIZE_BYTES);
        shadersBuffer.insert(shadersBuffer.end(), (uint8_t*)storageBuffer->getPointer() + CONTAINER_JSON_SIZE_BYTES + containerJsonSize, (uint8_t*)storageBuffer->getPointer() + storageBuffer->getSize());
    }
    for (auto& entry : m_container) {
        if (!entry.serialized) {
            assert(entry.value);
            entry.shaderParams.offset = shadersBuffer.size();
            shadersBuffer.reserve(shadersBuffer.size() + entry.shaderParams.codeByteSize);
            shadersBuffer.insert(shadersBuffer.end(), (uint8_t*)entry.value->getContent()->getPointer(), (uint8_t*)entry.value->getContent()->getPointer() + entry.shaderParams.codeByteSize);
        }
    }
    json containerJson;
    to_json(containerJson, m_container);
    std::string dumpedContainerJson = std::move(containerJson.dump());
    uint64_t dumpedContainerJsonLength = dumpedContainerJson.size();
    // first 8 entries are the size of the json, stored byte by byte
    std::vector<uint8_t> retVal;
    retVal.reserve(CONTAINER_JSON_SIZE_BYTES + dumpedContainerJsonLength + shadersBuffer.size());
    for (size_t i = 0; i < CONTAINER_JSON_SIZE_BYTES; i++) {
        uint8_t byte = static_cast<uint8_t>((dumpedContainerJsonLength >> (8 * (CONTAINER_JSON_SIZE_BYTES - i - 1))) & 0xFF);
        retVal.push_back(byte);
    }
    retVal.insert(retVal.end(), std::make_move_iterator(dumpedContainerJson.begin()), std::make_move_iterator(dumpedContainerJson.end()));
    retVal.insert(retVal.end(), std::make_move_iterator(shadersBuffer.begin()), std::make_move_iterator(shadersBuffer.end()));
    return retVal;
}

core::smart_refctd_ptr<IShaderCompiler::CCache> IShaderCompiler::CCache::deserialize(core::smart_refctd_ptr<ICPUBuffer> serializedCache)
{
    auto retVal = core::make_smart_refctd_ptr<CCache>();
    retVal->storageBuffer = std::move(serializedCache);
    // First get the size of the json in the buffer
    uint64_t* bufferStart = (uint64_t*)retVal->storageBuffer->getPointer();
    retVal->containerJsonSize = bufferStart[0];
    uint8_t* containerJsonStart = (uint8_t*)(bufferStart + 1);
    std::string containerJsonString(containerJsonStart, containerJsonStart + retVal->containerJsonSize);
    json containerJson = json::parse(containerJsonString);
    from_json(containerJson, retVal->m_container);
    
    return retVal;
}

core::smart_refctd_ptr<IShaderCompiler::CCache> IShaderCompiler::CCache::deserialize(std::span<uint8_t> serializedCache)
{
    auto cacheBuffer = core::make_smart_refctd_ptr<ICPUBuffer>(serializedCache.size());
    memcpy(cacheBuffer->getPointer(), serializedCache.data(), serializedCache.size());
    return IShaderCompiler::CCache::deserialize(cacheBuffer);
}
