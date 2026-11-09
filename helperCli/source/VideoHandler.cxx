#include "pre.hxx"

#include "VideoHandler.hxx"

#include "common.h"

bool VideoHandler::Handle(const std::string & command, std::string & filename)
{
    if (!string_starts_with(command, "/video ")) {
        return false;
    }
    filename = string_strip(command.substr(7));
    return true;
}
