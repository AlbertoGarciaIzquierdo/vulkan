#include "CommonIncludes.h"

using String = std::string;

template<typename... T>
using UniqPtr = std::unique_ptr<T...>;

template<typename... T>
using Vector = std::vector<T...>;

template<typename T>
using Optional = std::optional<T>;

template<typename... T>
using Set = std::set<T...>;