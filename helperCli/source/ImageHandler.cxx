#include "pre.hxx"

#include "ImageHandler.hxx"

#include "common.h"

bool ImageHandler::Handle(const std::string & command, std::string & filename)
{
    if (!string_starts_with(command, "/image ")) {
        return false;
    }
    filename = string_strip(command.substr(7));
    return true;
}
