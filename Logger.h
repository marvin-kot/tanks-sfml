#pragma once

#include <fstream>
#include <iostream>

class Logger
{
private:
    Logger() : out("logfile.txt")  {
        coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to logfile
    }

    ~Logger() {
        std::cout.rdbuf(coutbuf); //reset to standard output again
    }

public:
    Logger(Logger& l) = delete;
    void operator=(const Logger& l) = delete;

    static Logger& instance() {
        static Logger instance;
        
        return instance;
    }

    template <typename T>
    Logger& operator<<(T&& t) // provide a generic operator<<
    {
       out << t << std::endl;
       return *this;
    }


private:
    std::ofstream out;
    std::streambuf *coutbuf;

};
