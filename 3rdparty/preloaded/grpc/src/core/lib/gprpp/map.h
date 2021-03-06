/*
 *
 * Copyright 2017 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_CORE_LIB_GPRPP_MAP_H
#define GRPC_CORE_LIB_GPRPP_MAP_H

#include <grpc/support/port_platform.h>

#include <string.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>

#include "src/core/lib/gpr/useful.h"
#include "src/core/lib/gprpp/memory.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/gprpp/string_view.h"

namespace grpc_core {

#if PFS_HAVE_NOT_map_emplace
template <class Key, class T, class Compare = std::less<Key>>
class Map : public std::map<Key, T, Compare, Allocator<std::pair<const Key, T>>>
{
public:
    using base_class = std::map<Key, T, Compare, Allocator<std::pair<const Key, T>>>;
    using iterator = typename base_class::iterator;
public:
    template<typename K, typename U>
    std::pair<iterator, bool> emplace (K && key, U && value)
    {
        return this->insert(std::make_pair(std::forward<K>(key), std::forward<U>(value)));
    }
};
#else
template <class Key, class T, class Compare = std::less<Key>>
using Map = std::map<Key, T, Compare, Allocator<std::pair<const Key, T>>>;
#endif

struct StringLess {
  bool operator()(const char* a, const char* b) const {
    return strcmp(a, b) < 0;
  }
  bool operator()(const UniquePtr<char>& a, const UniquePtr<char>& b) const {
    return strcmp(a.get(), b.get()) < 0;
  }
  bool operator()(const StringView& a, const StringView& b) const {
    const size_t min_size = std::min(a.size(), b.size());
    int c = strncmp(a.data(), b.data(), min_size);
    if (c != 0) return c < 0;
    return a.size() < b.size();
  }
};

template <typename T>
struct RefCountedPtrLess {
  bool operator()(const RefCountedPtr<T>& p1,
                  const RefCountedPtr<T>& p2) const {
    return p1.get() < p2.get();
  }
};

}  // namespace grpc_core

#endif /* GRPC_CORE_LIB_GPRPP_MAP_H */
