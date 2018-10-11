/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <gtest/gtest.h>

#include <geode/CacheableString.hpp>
#include <geode/internal/functional.hpp>

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::dereference_equal_to;
using apache::geode::client::dereference_hash;

TEST(CacheableStringEqualityTest, StdHashSpecializationViaStdSharedPtr) {
  auto s = CacheableString::create("test");
  typedef decltype(s) key_type;
  EXPECT_EQ(s->hashcode(),
            static_cast<int32_t>(dereference_hash<key_type>{}(s)));
}

TEST(CacheableStringEqualityTest, StdHashSpecializationViaStdPtr) {
  auto s = CacheableString::create("test");
  typedef decltype(s)::element_type* key_type;

  EXPECT_EQ(s->hashcode(),
            static_cast<int32_t>(dereference_hash<key_type>{}(s.get())));
}

TEST(CacheableStringEqualityTest, CacheableEqualToCalled) {
  auto s1 = CacheableString::create("test");
  auto s2 = CacheableString::create("test");
  typedef decltype(s1) key_type;

  EXPECT_NE(s1, s2);
  EXPECT_EQ(*s1, *s2);
  EXPECT_TRUE(dereference_equal_to<key_type>{}(s1, s2));
}

TEST(CacheableStringEqualityTest, CacheableHashSet) {
  auto s1 = CacheableString::create("test");
  auto s2 = CacheableString::create("test");
  auto s3 = CacheableString::create("nope");
  typedef decltype(s1) key_type;

  EXPECT_NE(s1, s2);
  EXPECT_EQ(*s1, *s2);
  EXPECT_EQ(3387254, s3->hashcode());
  EXPECT_EQ(3556498, s1->hashcode());
  EXPECT_EQ(s1->hashcode(), s2->hashcode());

  std::unordered_set<key_type, dereference_hash<key_type>,
                     dereference_equal_to<key_type>>
      set = {s1};

  {
    const auto& f = set.find(s2);
    EXPECT_NE(set.end(), f);
    EXPECT_EQ(s1, *f);
    EXPECT_NE(s2, *f);
  }

  {
    const auto& f = set.find(s3);
    EXPECT_EQ(set.end(), f);
  }
}

TEST(CacheableStringEqualityTest, CacheableHashSetExplicitHash) {
  auto s1 = CacheableString::create("test");
  auto s2 = CacheableString::create("test");
  auto s3 = CacheableString::create("nope");
  typedef decltype(s1) key_type;

  EXPECT_NE(s1, s2);
  EXPECT_EQ(*s1, *s2);
  EXPECT_EQ(s1->hashcode(), s2->hashcode());

  std::unordered_set<key_type, CacheableKey::hash, CacheableKey::equal_to> set =
      {s1};

  {
    const auto& f = set.find(s2);
    EXPECT_NE(set.end(), f);
    EXPECT_EQ(s1, *f);
    EXPECT_NE(s2, *f);
  }

  {
    const auto& f = set.find(s3);
    EXPECT_EQ(set.end(), f);
  }
}

TEST(CacheableStringEqualityTest, CacheableHashMapViaSharedPtr) {
  auto s1 = CacheableString::create("test");
  auto s2 = CacheableString::create("test");
  auto s3 = CacheableString::create("nope");
  typedef decltype(s1) key_type;

  EXPECT_NE(s1, s2);
  EXPECT_EQ(*s1, *s2);
  EXPECT_EQ(s1->hashcode(), s2->hashcode());

  std::unordered_map<key_type, int, dereference_hash<key_type>,
                     dereference_equal_to<key_type>>
      map = {{s1, 1}};

  {
    const auto& f = map.find(s2);
    EXPECT_NE(map.end(), f);
    EXPECT_EQ(s1, f->first);
    EXPECT_NE(s2, f->first);
  }

  {
    const auto& f = map.find(s3);
    EXPECT_EQ(map.end(), f);
  }
}

TEST(CacheableStringEqualityTest, CacheableHashMapViaPtr) {
  auto s1 = CacheableString::create("test");
  auto s2 = CacheableString::create("test");
  auto s3 = CacheableString::create("nope");
  typedef decltype(s1)::element_type* key_type;

  EXPECT_NE(s1, s2);
  EXPECT_EQ(*s1, *s2);
  EXPECT_EQ(s1->hashcode(), s2->hashcode());

  std::unordered_map<key_type, int, dereference_hash<key_type>,
                     dereference_equal_to<key_type>>
      map = {{s1.get(), 1}};

  {
    const auto& f = map.find(s2.get());
    EXPECT_NE(map.end(), f);
    EXPECT_EQ(s1.get(), f->first);
    EXPECT_NE(s2.get(), f->first);
  }

  {
    const auto& f = map.find(s3.get());
    EXPECT_EQ(map.end(), f);
  }
}

TEST(CacheableStringEqualityTest, CacheableHashMapExplicitHash) {
  auto s1 = CacheableString::create("test");
  auto s2 = CacheableString::create("test");
  auto s3 = CacheableString::create("nope");
  typedef decltype(s1) key_type;

  EXPECT_NE(s1, s2);
  EXPECT_EQ(*s1, *s2);
  EXPECT_EQ(s1->hashcode(), s2->hashcode());

  std::unordered_map<key_type, int, CacheableKey::hash, CacheableKey::equal_to>
      map = {{s1, 1}};

  {
    const auto& f = map.find(s2);
    EXPECT_NE(map.end(), f);
    EXPECT_EQ(s1, f->first);
    EXPECT_NE(s2, f->first);
  }

  {
    const auto& f = map.find(s3);
    EXPECT_EQ(map.end(), f);
  }
}
