#pragma once
#include <oead/sarc.h>
#include <oead/yaz0.h>
#include <oead/byml.h>
#include <filesystem>
#include <fstream>
#include <utility>
#include <memory>
#include <string>
#include <vector>

namespace Utils 
{

namespace Sarc {

using Byml = oead::Byml;
using BymlVec = std::vector<std::pair<std::string_view, Byml>>;
using SarcWriter = oead::SarcWriter;
namespace yaz0 = oead::yaz0;

auto DumpBymls(const std::filesystem::path& p) {
    auto path = std::filesystem::absolute(p);
    auto instream = std::unique_ptr<std::ifstream>(new std::ifstream{path, std::ios::binary});
    path = path.parent_path() / path.stem();
    std::filesystem::create_directories(path);
    std::vector<u8> vec = {std::istreambuf_iterator<char>{*instream}, {}};
    instream->close();
    auto magic = std::string((char*)vec.data(), 4);
    vec = magic == "Yaz0" ? yaz0::Decompress(vec) : vec;
    auto sarc = oead::Sarc(vec);
    auto bymls = BymlVec();
    for (const auto& file : sarc.GetFiles()) {
        if (file.name.ends_with(".byml")) {
            // file.name is copied.
            bymls.push_back(std::make_pair(file.name, Byml::FromBinary(file.data)));
        }
    }
    auto result = std::vector<std::filesystem::path>();
    for (const auto& [name, file] : bymls) {
        auto fullname = path / name;
        fullname.replace_extension(".yml");
        try {
            auto text = file.ToText();
            auto outstream = std::unique_ptr<std::ofstream>(new std::ofstream{ fullname });
            outstream->write(text.data(), text.size());
            outstream->close();
            // It is safe to move fullname at this point.
            result.push_back(std::move(fullname));
        }
        catch (...) {   }
    }
    return result;
}

auto ReplaceBymls(const std::filesystem::path& p, int version = 2, int level = 7) {
    auto path = std::filesystem::absolute(p);
    auto folder = path.has_extension() ? path.stem() : path;
    path.replace_extension(".szs");
    auto files = std::vector<std::filesystem::path>();
    auto iter = std::filesystem::directory_iterator(folder);
    for (auto& item : iter) {
        if (item.is_regular_file() && item.path().string().ends_with(".yml")) {
            // This will copy.
            files.push_back(item.path());
        } 
    }
    auto instream = std::unique_ptr<std::ifstream>(new std::ifstream{path, std::ios::binary});
    auto vector = std::vector<u8>(std::istreambuf_iterator<char>{*instream}, {});
    instream->close();
    vector = std::string((char*)vector.data(), 4) == "Yaz0" ? oead::yaz0::Decompress(vector) : vector;
    auto sarc = oead::Sarc(vector);
    auto writer = SarcWriter::FromSarc(sarc);
    for (const auto& file : files) {
        auto name = file.filename().replace_extension(".byml").string();
        if (writer.m_files.find(name) != writer.m_files.end()) {
            auto istream = std::unique_ptr<std::ifstream>(new std::ifstream{file});
            auto str = std::string(std::istreambuf_iterator<char>{*istream}, {});
            istream->close();
            auto byml = Byml::FromText(str);
            auto& vec = writer.m_files[name];
            // Move new data into vec.
            vec = byml.ToBinary(sarc.GetEndianness() == oead::util::Endianness::Big, version);
        }
    }
    auto data = writer.Write();
    // Alignment value from the pair *could* be used, 
    // but uhhhh idk if it's good to use with the yaz0.
    data.second = yaz0::Compress(data.second, 0, level);
    auto outstream = std::unique_ptr<std::ofstream>(new std::ofstream{ path, std::ios::binary });
    outstream->write((char*)data.second.data(), data.second.size());
    outstream->close();
}

}

}