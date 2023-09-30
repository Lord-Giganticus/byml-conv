#include "sarc_handler.hpp"
#include "byml_handler.hpp"
#include <string>
#include <iostream>
#include <filesystem>
#include <boost/program_options.hpp>
#include <oead/util/swap.h>

namespace po = boost::program_options;
namespace fs = std::filesystem;

const int sys_endian = static_cast<int>(oead::util::detail::GetPlatformEndianness());

int main(int argc, char* argv[]) {
    int endian = sys_endian;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("input", po::value<std::string>(), "The input file/folder to provide.")
        ("endian", po::value<int>(&endian), "Enter 0 for Big Endian, 1 for Little Endian.");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help") || vm.size() == 0) {
        std::cout << desc << std::endl;
        return 0;
    }
    if (vm.count("input")) {
        std::filesystem::path path = vm["input"].as<std::string>();
        if ((path.extension() == ".szs" || path.extension() == ".sarc") && fs::is_regular_file(path)) {
            const auto paths = Utils::Sarc::DumpBymls(path);
            std::cout << "Dumped the following file(s): " << std::endl;
            for (const auto& p : paths) {
                std::cout << path.stem() / p.filename() << std::endl;
            }
        }
        else if (path.extension() == ".byml" && fs::is_regular_file(path)) {
            Utils::Byml::DumpData(path);
            std::cout << "Dumped the byml data to: " << path.replace_extension(".yml") << std::endl;
        }
        else if (path.extension() == ".yml" && fs::is_regular_file(path)) {
            Utils::Byml::ReplaceData(path, static_cast<oead::util::Endianness>(endian));
            std::cout << "Saved the byml data to: " << path.replace_extension(".byml") << std::endl;
        }
        else if (fs::is_directory(path)) {
            Utils::Sarc::ReplaceBymls(path);
            std::cout << "Saved the sarc to: " << path.replace_extension(".szs") << std::endl;
        }
    }
}