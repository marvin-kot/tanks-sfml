#pragma once

#include <fstream>
#include <iostream>

class Logger
{
private:
    Logger() : out()  {
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
        static Logger _instance;
        return _instance;
    }

    template <typename T>
    Logger& operator<<(T&& t) // provide a generic operator<<
    {
        if (!filenameSpecified)
            setFilename("logfile.txt");
       out << t;
       return *this;
    }

    void setFilename(std::string filename)
    {
        out.open(filename);
        filenameSpecified = true;
    }

private:
    std::ofstream out;
    std::streambuf *coutbuf;

    bool filenameSpecified = false;

};
