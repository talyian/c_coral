#include "zero_types.hh"
#include "zero_ast.hh"
#include "zero_json.hh"
#include "zero_codegen.hh"

#include "lib/cxxopts.hpp"

#include <string>
#include <iostream>
#include <fstream>

int main(int argc, const char ** argv) {
  std::cout << "Coral Zero\n";
  std::cout.flush();
  cxxopts::Options parser("zero", "Coral Zero Compiler");
  parser.add_options()
    ("f,file", "Main Source File", cxxopts::value<std::string>());
  parser.parse_positional({"file"});
  auto args = parser.parse(argc, argv);
  if (args.count("file")) {
    std::cout << "File: [" << args["file"].as<std::string>() << "]\n";
    auto filename = args["file"].as<std::string>();
    std::ifstream file;
    file.open(filename);
    coral_zero::JsonParser::parse_file(file);
  }
  else
    coral_zero::JsonParser::parse_file(std::cin);
  return 0;
}
