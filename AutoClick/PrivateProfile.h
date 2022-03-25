#pragma once
#include <vector>
#include "tstring.h"

struct PrivateProfile
{
    struct KeyAndValue
    {
    public:
        std::_tstring Key;
        std::_tstring Value;
    };

    struct Section
    {
    public:
        std::_tstring Name;
        std::vector<KeyAndValue> Values;
    };

public:
    std::vector<Section> Sections;
};
