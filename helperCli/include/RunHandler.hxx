#ifndef _RUNHANDLER_HXX_
#define _RUNHANDLER_HXX_

#include <cstddef>
#include <string>
#include <vector>

class RunHandler
{
public:
    std::string Handle(const std::string & args);
    static std::vector<std::string> Complete(std::string & line, size_t & cursor, size_t command_start);
    const std::string & Status() const { return status; }
    bool Succeeded() const { return succeeded; }

private:
    std::string status;
    bool succeeded = false;
};

#endif // _RUNHANDLER_HXX_
