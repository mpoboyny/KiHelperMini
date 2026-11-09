#include "pre.hxx"

#include "ClearHandler.hxx"

#include "common.h"

bool ClearHandler::Handle(const std::string & command)
{
    return string_starts_with(command, "/clear");
}
