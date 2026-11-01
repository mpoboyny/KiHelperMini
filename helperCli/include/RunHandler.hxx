#pragma once

#include <string>

class RunHandler
{
public:
    std::string Handle(const std::string & args);
    const std::string & Status() const { return status; }
    bool Succeeded() const { return succeeded; }

private:
    std::string status;
    bool succeeded = false;
};
