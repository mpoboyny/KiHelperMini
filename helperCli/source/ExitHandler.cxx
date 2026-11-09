#include "pre.hxx"

#include "ExitHandler.hxx"

#include "common.h"

bool ExitHandler::Handle(const std::string & command)
{
    return string_starts_with(command, "/exit");
}
