#include "pre.hxx"

#include "RegenerateHandler.hxx"

#include "common.h"

bool RegenerateHandler::Handle(const std::string & command)
{
    return string_starts_with(command, "/regen");
}
