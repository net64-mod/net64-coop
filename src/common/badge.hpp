//
// Created by henrik on 20.06.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once


template<typename T>
struct Badge
{
    friend T;

private:
    Badge() {}
};
