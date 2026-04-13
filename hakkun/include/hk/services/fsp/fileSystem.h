#pragma once

#include "hk/Result.h"
#include "hk/container/StringView.h"
#include "hk/services/fsp/file.h"
#include "hk/services/fsp/util.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"

namespace hk::fsp {
    enum FileMode {
        FileMode_Read = bit(0),
        FileMode_Write = bit(1),
        FileMode_Append = bit(2),
    };

    enum DirectoryFilter {
        DirectoryFilter_Directories = bit(0),
        DirectoryFilter_Files = bit(1),
    };

    class IFileSystem : public sf::Service {
    public:
        IFileSystem(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        Result createFile(StringView path, s64 size, u32 flags) {
            ASSERT_PATH_LEN(path);
            auto input = sf::packInput(flags, size);
            auto request = sf::Request(this, 0, &input);
            request.addInPointer<char>(path);
            return invokeRequest(move(request));
        }

        Result deleteFile(StringView path) {
            ASSERT_PATH_LEN(path);
            auto request = sf::Request(this, 1);
            request.addInPointer<char>(path);
            return invokeRequest(move(request));
        }

        Result createDirectory(StringView path) {
            ASSERT_PATH_LEN(path);
            auto request = sf::Request(this, 2);
            request.addInPointer<char>(path);
            return invokeRequest(move(request));
        }

        Result deleteDirectory(StringView path) {
            ASSERT_PATH_LEN(path);
            auto request = sf::Request(this, 3);
            request.addInPointer<char>(path);
            return invokeRequest(move(request));
        }

        Result deleteDirectoryRecursive(StringView path) {
            ASSERT_PATH_LEN(path);
            auto request = sf::Request(this, 4);
            request.addInPointer<char>(path);
            return invokeRequest(move(request));
        }

        Result renameFile(StringView oldPath, StringView newPath) {
            ASSERT_PATH_LEN(oldPath);
            ASSERT_PATH_LEN(newPath);
            auto request = sf::Request(this, 5);
            request.addInPointer<char>(oldPath);
            request.addInPointer<char>(newPath);
            return invokeRequest(move(request));
        }

        Result renameDirectory(StringView oldPath, StringView newPath) {
            ASSERT_PATH_LEN(oldPath);
            ASSERT_PATH_LEN(newPath);
            auto request = sf::Request(this, 6);
            request.addInPointer<char>(oldPath);
            request.addInPointer<char>(newPath);
            return invokeRequest(move(request));
        }

        ValueOrResult<IFile> openFile(StringView path, FileMode mode) {
            ASSERT_PATH_LEN(path);
            auto request = sf::Request(this, 8, &mode);
            request.addInPointer<char>(path);
            return IFile(HK_TRY(invokeRequest(move(request), sf::subserviceExtractor(this))));
        }

        ValueOrResult<sf::Service> openDirectory(StringView path, DirectoryFilter filter) {
            ASSERT_PATH_LEN(path);
            auto request = sf::Request(this, 9, &filter);
            request.addInPointer<char>(path);
            return invokeRequest(move(request), sf::subserviceExtractor(this));
        }

        // commits save data to disk
        Result commit() {
            return sf::invokeSimple(this, 10);
        }
    };
} // namespace hk::fsp
