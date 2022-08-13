#ifndef _LDEBUG_H
#define _LDEBUG_H
#include <iostream>
#include <string>
namespace LightOi
{
    class LDebug
    {
        public:
            static void ldebug(std::string str,int number = 0)
            {
                #ifdef DEBUG_COUT
                    std::cout << str << std::endl;
                #endif
            }   
    };
}

#endif