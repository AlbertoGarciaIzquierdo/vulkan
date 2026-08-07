#include <Engine/Utils/CommonIncludes.h>

using Text = std::string;
using String = const char*;

template<typename... T>
using UniqPtr = std::unique_ptr<T...>;
template<typename... T>
using SharedPtr = std::shared_ptr<T...>;

template<typename... T>
using Vector = std::vector<T...>;

template<typename T>
using Optional = std::optional<T>;

template<typename... T>
using Set = std::set<T...>;

using UInt32 = uint32_t;
using UInt64 = uint64_t;
using Int32 = int32_t;
using Int64 = int64_t;