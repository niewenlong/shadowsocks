#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <iostream>
#include "shadowsocks/ss_core.h"


// print program information and license to screen
std::ostream& Ss_Core::printShadowSocksHeader(std::ostream &s) {
    s << "ShadowSocks-" << SHADOWSOCKS_MODULE << " "
      << "Ver: " << SHADOWSOCKS_VERSION << std::endl;
    s << "Copyright (C) 2018 Jayson Wang <jayson@shellboot.com>" << std::endl;
    s << "MIT License: " << "<" SHADOWSOCKS_LICENSE_LINK ">" << std::endl;
    s << std::endl;

    return s;
}

// get last error from system
std::string &&Ss_Core::getLastError() {
    std::stringstream ss;
    ss << "Error<" << errno << ">: " << strerror(errno);

    auto s = new std::string(ss.str());
    return std::move(*s);
}

// print last error and user message
void Ss_Core::printLastError(std::string &additionalMessage) {
    std::flush(std::cout);
    std::cerr << getLastError() << " <UserReason>: " << additionalMessage;
}

// overload by printLastError
void Ss_Core::printLastError(const char *additionalMessage) {
    std::string msg(additionalMessage);
    printLastError(msg);
}