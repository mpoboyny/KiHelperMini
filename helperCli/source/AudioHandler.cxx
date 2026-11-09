#include "pre.hxx"

#include "AudioHandler.hxx"

#include "common.h"

bool AudioHandler::Handle(const std::string & command, std::string & filename)
{
    if (!string_starts_with(command, "/audio ")) {
        return false;
    }
    filename = string_strip(command.substr(7));
    return true;
}
