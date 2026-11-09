#ifndef _READHANDLER_HXX_
#define _READHANDLER_HXX_

class ReadHandler
{
public:
    static bool Handle(const std::string & command, std::string & filename);
};

#endif // _READHANDLER_HXX_
