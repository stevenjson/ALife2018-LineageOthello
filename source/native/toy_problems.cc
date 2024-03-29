// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "base/vector.h"

#include "config/command_line.h"
#include "config/ArgManager.h"

#include "../toy-config.h"
#include "../ToyProblemExp.h"

int main(int argc, char* argv[])
{
  // Read configs.
  std::string config_fname = "configs.cfg";
  auto args = emp::cl::ArgManager(argc, argv);
  ToyConfig config;
  config.Read(config_fname);

  if (args.ProcessConfigOptions(config, std::cout, config_fname, "../toy-config.h") == false)
    exit(0);
  if (args.TestUnknown() == false)
    exit(0);

  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  config.Write(std::cout);
  std::cout << "==============================\n"
            << std::endl;

  ToyProblemExp e(config);
  e.Run();
}
