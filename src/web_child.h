#ifndef WEB_CHILD_H
#define WEB_CHILD_H

#include <nlohmann/json.hpp>

using json = nlohmann::json;

int open_web_process();
void close_web_process();
void write_to_web_process(const json &jsono);

#endif // WEB_CHILD_H
