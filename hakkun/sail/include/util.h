#pragma once

#include <deque>
#include <filesystem>
#include <fstream>
#include <ios>
#include <span>
#include <string>
#include <vector>

namespace sail {

    template <typename T>
    void writeFile(const std::string& filename, std::span<T> data) {
        std::ofstream fstr(filename, std::ios::binary);
        fstr.unsetf(std::ios::skipws);
        fstr.write((const char*)data.data(), data.size() * sizeof(T));
    }

    template <int ExitCode = 1, typename... Args>
    [[noreturn]] void fail(const char* msg, Args... args) {
        fprintf(stderr, msg, args...);
        exit(ExitCode);
    }

    template <typename T>
    std::vector<T> readFile(const char* filename) {
        if (!std::filesystem::exists(filename))
            fail<2>("%s does not exist\n", filename);

        std::ifstream fstr(filename, std::ios::binary);
        fstr.unsetf(std::ios::skipws);

        std::streampos fileSize;

        fstr.seekg(0, std::ios::end);
        fileSize = fstr.tellg();
        fstr.seekg(0, std::ios::beg);

        std::vector<T> data;
        data.resize(fileSize);
        fstr.read((char*)data.data(), fileSize);

        return data;
    }

    inline std::string readFileString(const char* filename) {
        if (!std::filesystem::exists(filename))
            fail<2>("%s does not exist\n", filename);

        std::ifstream t(filename);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }

    inline std::deque<std::string> split(const std::string& str, const char delim) {
        std::stringstream ss(str);
        std::string s;

        std::deque<std::string> out;

        while (std::getline(ss, s, delim))
            if (!s.empty())
                out.push_back(s);

        return out;
    }

    inline bool hexStringToBytes(std::vector<uint8_t>& out, const std::string& hex) {
        size_t length = hex.length();

        if (length % 2 != 0)
            return false;

        for (size_t i = 0; i < length; i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            out.push_back(byte);
        }

        return true;
    }

} // namespace sail
