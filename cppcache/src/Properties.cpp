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

#include <ace/Hash_Map_Manager.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <ace/config-lite.h>
#include <ace/Versioned_Namespace.h>
#include <ace/OS_NS_stdio.h>

#include <geode/Properties.hpp>
#include <geode/GeodeTypeIds.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

template <>
class ACE_Hash<std::shared_ptr<apache::geode::client::CacheableKey>> {
 public:
  u_long operator()(
      const std::shared_ptr<apache::geode::client::CacheableKey>& key) {
    return key->hashcode();
  }
};

template <>
class ACE_Equal_To<std::shared_ptr<apache::geode::client::CacheableKey>> {
 public:
  int operator()(
      const std::shared_ptr<apache::geode::client::CacheableKey>& lhs,
      const std::shared_ptr<apache::geode::client::CacheableKey>& rhs) const {
    return (*lhs.get() == *rhs.get());
  }
};
ACE_END_VERSIONED_NAMESPACE_DECL

namespace apache {
namespace geode {
namespace client {

typedef ACE_Hash_Map_Manager_Ex<
    std::shared_ptr<CacheableKey>, std::shared_ptr<Cacheable>,
    ACE_Hash<std::shared_ptr<CacheableKey>>,
    ACE_Equal_To<std::shared_ptr<CacheableKey>>, ACE_Recursive_Thread_Mutex>
    CacheableKeyCacheableMap;

typedef ACE_Guard<ACE_Recursive_Thread_Mutex> CacheableKeyCacheableMapGuard;

#define MAP ((CacheableKeyCacheableMap*)m_map)

class PropertiesFile {
  Properties& m_props;
  std::string m_fileName;

 public:
  explicit PropertiesFile(Properties& props);

  void parseLine(const std::string& line);
  void readFile(const std::string& fileName);
  char* nextBufferLine(char** cursor, char* bufbegin, size_t totalLen);
};
std::shared_ptr<Properties> Properties::create() {
  return std::shared_ptr<Properties>(new Properties());
}

Properties::Properties() : Serializable() {
  m_map = (void*)new CacheableKeyCacheableMap();
  MAP->open();
}
Properties::~Properties() noexcept {
  if (m_map != nullptr) {
    delete MAP;
    m_map = nullptr;
  }
}

std::shared_ptr<CacheableString> Properties::find(const std::string& key) {
  auto keyptr = CacheableString::create(key.c_str());
  CacheableKeyCacheableMapGuard guard(MAP->mutex());
  std::shared_ptr<Cacheable> value;
  MAP->find(keyptr, value);
  return std::dynamic_pointer_cast<CacheableString>(value);
}
std::shared_ptr<Cacheable> Properties::find(
    const std::shared_ptr<CacheableKey>& key) {
  if (key == nullptr) {
    throw NullPointerException("Properties::find: Null key given.");
  }
  CacheableKeyCacheableMapGuard guard(MAP->mutex());
  std::shared_ptr<Cacheable> value;
  MAP->find(key, value);
  return value;
}

void Properties::insert(std::string key, std::string value) {
  auto keyptr = CacheableString::create(std::move(key));
  auto valptr = CacheableString::create(std::move(value));
  MAP->rebind(keyptr, valptr);
}

void Properties::insert(std::string key, const int value) {
  auto keyptr = CacheableString::create(std::move(key));
  auto valptr = CacheableString::create(std::to_string(value));
  MAP->rebind(keyptr, valptr);
}

void Properties::insert(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Cacheable>& value) {
  if (key == nullptr) {
    throw NullPointerException("Properties::insert: Null key given.");
  }
  MAP->rebind(key, value);
}

void Properties::remove(const std::string& key) {
  auto keyptr = CacheableString::create(key.c_str());
  MAP->unbind(keyptr);
}

void Properties::remove(const std::shared_ptr<CacheableKey>& key) {
  MAP->unbind(key);
}

size_t Properties::getSize() const { return MAP->current_size(); }

void Properties::foreach (Visitor& visitor) const {
  CacheableKeyCacheableMapGuard guard(MAP->mutex());

  for (auto&& entry : *MAP) {
    visitor.visit(entry.ext_id_, entry.int_id_);
  }
}

void Properties::addAll(const std::shared_ptr<Properties>& other) {
  if (other == nullptr) return;

  class Copier : public Visitor {
    Properties& m_lhs;

   public:
    explicit Copier(Properties& lhs) : m_lhs(lhs) {}
    void visit(const std::shared_ptr<CacheableKey>& key, const std::shared_ptr<Cacheable>& value) {
      m_lhs.insert(key, value);
    }
  } aCopier(*this);

  other->foreach (aCopier);
}

void Properties::load(const std::string& fileName) {
  PropertiesFile pf(*this);
  pf.readFile(fileName);
}

PropertiesFile::PropertiesFile(Properties& props) : m_props(props) {}

void PropertiesFile::readFile(const std::string& fileName) {
  char buf[8192];
  FILE* fp = fopen(fileName.c_str(), "r");
  if (fp != nullptr) {
    size_t len = 0;
    while ((len = fread(buf, 1, 8192, fp)) != 0) {
      /* adongre
       * CID 28894: String not null terminated (STRING_NULL)
       * Function "fread" does not terminate string "*buf".
       */
      buf[len] = '\0';
      char* tmp = buf;
      char* line = 0;
      while ((line = nextBufferLine(&tmp, buf, len)) != nullptr) {
        parseLine(line);
      }
    }
    fclose(fp);
  }
}

char* PropertiesFile::nextBufferLine(char** cursor, char* bufbegin,
                                     size_t totalLen) {
  if (static_cast<size_t>((*cursor) - bufbegin) >= totalLen) {
    return 0;
  }

  char* lineBegin = *cursor;
  while (((**cursor) != 10) && ((**cursor) != 13) &&
         (static_cast<size_t>((*cursor) - bufbegin) < totalLen)) {
    (*cursor)++;
  }
  **cursor = '\0';
  (*cursor)++;  // move to begin of next line.
  return lineBegin;
}

void PropertiesFile::parseLine(const std::string& line) {
  if (line.c_str()[0] == '#') {
    return;
  }
  if (line.c_str()[0] == '/') {
    return;
  }

  char key[1024];
  char value[1024];
  int res = sscanf(line.c_str(), "%[^ \t=]%*[ \t=]%[^\n\r]", key, value);
  if (res != 2) {
    // bad parse...
    return;
  }
  // clean up value from trailing whitespace.
  size_t len = strlen(value);
  size_t end = len - 1;
  while (end > 0) {
    if (value[end] != ' ') {
      break;
    }
    value[end] = '\0';
    end--;
  }
  // clean up escape '\' sequences.
  // size_t idx = 0;
  // size_t widx = 0;
  // len = strlen( value );
  // while( idx < (len + 1 /* copy the Null terminator also */ ) ) {
  //  if ( value[idx] == '\\' ) {
  //    idx++;
  //  }
  //  value[widx] = value[idx];
  //  widx++;
  //  idx++;
  //}

  m_props.insert(key, value);
}

std::shared_ptr<Serializable> Properties::createDeserializable() {
  return std::make_shared<Properties>();
}

int32_t Properties::classId() const { return 0; }

int8_t Properties::typeId() const { return GeodeTypeIds::Properties; }

void Properties::toData(DataOutput& output) const {
  CacheableKeyCacheableMapGuard guard(MAP->mutex());
  int32_t mapSize = getSize();
  output.writeArrayLen(mapSize);
  CacheableKeyCacheableMap::iterator iter = MAP->begin();
  while (iter != MAP->end()) {
    output.writeObject((*iter).ext_id_);
    output.writeObject((*iter).int_id_);
    ++iter;
  }
}

void Properties::fromData(DataInput& input) {
  int32_t mapSize = input.readArrayLen();
  for (int i = 0; i < mapSize; i++) {
    auto key = std::static_pointer_cast<CacheableKey>(input.readObject());
    auto val = std::static_pointer_cast<Cacheable>(input.readObject());
    MAP->rebind(key, val);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
