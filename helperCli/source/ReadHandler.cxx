#include "pre.hxx"

#include "ReadHandler.hxx"

#include "common.h"

bool ReadHandler::Handle(const std::string & command, std::string & filename)
{
    if (!string_starts_with(command, "/read ")) {
        return false;
    }
    filename = string_strip(command.substr(6));
    if (filename.size() >= 2 && filename.front() == '"' && filename.back() == '"') {
        filename = filename.substr(1, filename.size() - 2);
    }
    return true;
}
