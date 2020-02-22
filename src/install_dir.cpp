//
// Created by henrik on 21.02.20.
//

#include <cstdlib>
#include "filesystem.hpp"


static const char* install_root()
{
    // Check if running inside an AppImage
    if(std::getenv("APPIMAGE") == nullptr)
    {
        return "../..";
    }
    else
    {
        return std::getenv("APPDIR");
    }
}

fs::path install_dir()
{
    static const char* install_root_s{install_root()};

    return fs::path{install_root_s} / "usr/share/net64";
}
