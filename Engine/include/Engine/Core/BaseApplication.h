//
// Created by alberto on 4/8/26.
//

#pragma once

/**
 * @file BaseApplication.h
 * @brief Abstract interface every application built on the engine implements.
 */

/**
 * @class BaseApplication
 * @brief Minimal application interface with a single entry point.
 */
class BaseApplication
{
    public:
    BaseApplication() = default;
    virtual ~BaseApplication() = default;

    /** @brief Runs the application until it finishes. */
    virtual void Run() = 0;
};
