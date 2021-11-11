#pragma once

#ifndef GEODE_SQLITEIMPL_SQLITEHELPER_H_
#define GEODE_SQLITEIMPL_SQLITEHELPER_H_

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
#pragma once

#include "sqlite3.h"
#include <geode/PersistenceManager.hpp>
#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/stat.h>
#endif

class SqLiteHelper {
 public:
  int initDB(const char* regionName, int maxPageCount, int pageSize,
             const char* regionDBfile, int busy_timeout_ms = 5000);
  int insertKeyValue(void* keyData, int keyDataSize, void* valueData,
                     int valueDataSize);
  int removeKey(void* keyData, int keyDataSize);
  int getValue(void* keyData, int keyDataSize, void*& valueData,
               int& valueDataSize);
  int closeDB();

 private:
  sqlite3* m_dbHandle;

  const char* m_tableName;
  int dropTable();
  int createTable();
  int executePragma(const char* pragmaName, int pragmaValue);
};

#endif  // GEODE_SQLITEIMPL_SQLITEHELPER_H_
