#include <openssl/ssl.h>
#include <libstatus.h>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>

int main()
{
    std::string str = "Hello World";
    char* c = const_cast<char*>(str.c_str());
    std::cout << HashMessage(c) << std::endl;
    return 0;
}


