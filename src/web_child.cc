#include "web_child.h"
#include <boost/process.hpp>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace bp = boost::process;

namespace {
std::unique_ptr<bp::child> web_process;
std::unique_ptr<bp::opstream> process_stream;
} // namespace

int open_web_process() {
  if (!web_process || !web_process->running()) {
    process_stream = std::make_unique<bp::opstream>();
    web_process = std::make_unique<bp::child>("./web/server",
                                              bp::std_in < *process_stream);
    if (!web_process->running()) {
      std::cerr << "Failed to start web child process!" << std::endl;
      process_stream.reset();
      web_process.reset();
      return 1;
    }
  }
  return 0;
}

void close_web_process() {
  if (web_process && web_process->running()) {
    process_stream.reset();
    web_process->terminate();
    web_process->wait();
    web_process.reset();
  }
}

void write_to_web_process(const json &jsono) {
  if (!process_stream) {
    std::cerr << "web child process is not open!" << std::endl;
    return;
  }
  *process_stream << jsono.dump() << std::endl;
}
