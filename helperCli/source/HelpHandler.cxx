#include "pre.hxx"
#include "HelpHandler.hxx"

bool HelpHandler::Handle(const std::string & command)
{
    return command == "/help";
}
