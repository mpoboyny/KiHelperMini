#include "pre.hxx"

#include "GlobHandler.hxx"

#include "common.h"

bool GlobHandler::Handle(const std::string & command, std::string & pattern)
{
    if (!string_starts_with(command, "/glob ")) {
        return false;
    }
    pattern = string_strip(command.substr(6));
    return true;
}
