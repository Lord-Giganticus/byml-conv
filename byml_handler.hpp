#pragma once
#include <oead/byml.h>
#include <oead/util/swap.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

namespace Utils 
{


namespace Byml 
{

using Endianess = oead::util::Endianness;

auto DumpData(const std::filesystem::path& p) {
    auto path = std::filesystem::absolute(p);
    auto instream = std::unique_ptr<std::ifstream>(new std::ifstream{path, std::ios::binary});
    auto vec = std::vector<u8>(std::istreambuf_iterator<char>{*instream}, {});
    instream->close();
    auto byml = oead::Byml::FromBinary(vec);
    auto text = byml.ToText();
    path.replace_extension(".yml");
    auto outstream = std::unique_ptr<std::ofstream>(new std::ofstream{path});
    outstream->write(text.data(), text.size());
    outstream->close();
}

auto ReplaceData(const std::filesystem::path& p, Endianess endian, int version = 2) {
    auto path = std::filesystem::absolute(p);
    auto instream = std::unique_ptr<std::ifstream>(new std::ifstream{path});
    auto string = std::string(std::istreambuf_iterator<char>{*instream}, {});
    instream->close();
    auto byml = oead::Byml::FromText(string);
    auto data = byml.ToBinary(endian == Endianess::Big, version);
    path.replace_extension(".byml");
    auto outstream = std::unique_ptr<std::ofstream>(new std::ofstream{path, std::ios::binary});
    outstream->write((char*)data.data(), data.size());
    outstream->close();
}

}

}