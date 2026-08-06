//
// Created by alberto on 4/8/26.
//

#pragma once

class BaseApplication
{
    public:
    BaseApplication() = default;
    virtual ~BaseApplication() = default;

    virtual void Run() = 0;
};
