#pragma once

#include "2dfmFile.hpp"
#include "CommonResource.hpp"

struct KgtDemo : CommonResource {
    std::string demoName;
    _2dfm::KgtDemoConfig config {};
};
