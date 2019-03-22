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

#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Properties.hpp>

namespace apache {
namespace geode {
namespace client {

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
  return std::make_shared<Properties>();
}

std::shared_ptr<CacheableString> Properties::find(const std::string& key) {
  return std::dynamic_pointer_cast<CacheableString>(
      this->find(CacheableString::create(key)));
}

std::shared_ptr<Cacheable> Properties::find(
    const std::shared_ptr<CacheableKey>& key) {
  if (!key) {
    throw NullPointerException("Properties::find: Null key given.");
  }

  auto entry = m_map.find(key);
  if (entry != m_map.end()) {
    return entry->second;
  }

  return nullptr;
}

void Properties::insert(std::string key, std::string value) {
  this->insert(CacheableString::create(std::move(key)),
               CacheableString::create(std::move(value)));
}

void Properties::insert(std::string key, const int value) {
  this->insert(CacheableString::create(std::move(key)),
               CacheableString::create(std::to_string(value)));
}

void Properties::insert(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Cacheable>& value) {
  if (!key) {
    throw NullPointerException("Properties::insert: Null key given.");
  }

  m_map[key] = value;
}

void Properties::remove(const std::string& key) {
  this->remove(CacheableString::create(key));
}

void Properties::remove(const std::shared_ptr<CacheableKey>& key) {
  m_map.erase(key);
}

size_t Properties::getSize() const { return m_map.size(); }

void Properties::foreach (Visitor& visitor) const {
  for (auto& entry : m_map) {
    visitor.visit(entry.first, entry.second);
  }
}

void Properties::addAll(const std::shared_ptr<Properties>& other) {
  if (other == nullptr) return;

  for (auto& entry : other->m_map) {
    m_map[entry.first] = entry.second;
  }
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
      char* line = nullptr;
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
    return nullptr;
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

DSCode Properties::getDsCode() const { return DSCode::Properties; }

void Properties::toData(DataOutput& output) const {
  output.writeArrayLen(static_cast<uint32_t>(m_map.size()));
  for (auto& entry : m_map) {
    output.writeObject(entry.first);
    output.writeObject(entry.second);
  }
}

void Properties::fromData(DataInput& input) {
  auto mapSize = input.readArrayLength();
  m_map.reserve(mapSize);

  for (int i = 0; i < mapSize; i++) {
    auto key = std::dynamic_pointer_cast<CacheableKey>(input.readObject());
    auto value = std::dynamic_pointer_cast<Cacheable>(input.readObject());
    m_map.emplace(key, value);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
