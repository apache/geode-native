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

#include "SqLiteHelper.hpp"

#include <string.h>

#define QUERY_SIZE 512

int SqLiteHelper::initDB(const char* regionName, int maxPageCount, int pageSize,
                         const char* regionDBfile, int busy_timeout_ms) {
  // open the database
  int retCode = sqlite3_open(regionDBfile, &m_dbHandle);
  if (retCode == SQLITE_OK) {
    // set region name to  tablename. database name is also table name
    m_tableName = regionName;
    sqlite3_busy_timeout(m_dbHandle, busy_timeout_ms);

    // configure max page count
    if (maxPageCount > 0) {
      retCode = executePragma("max_page_count", maxPageCount);
    }

    if (retCode == SQLITE_OK && pageSize > 0) {
      retCode = executePragma("page_size", pageSize);
    }

    // create table
    if (retCode == SQLITE_OK) retCode = createTable();
  }

  return retCode;
}

int SqLiteHelper::createTable() {
  // construct query
  char query[QUERY_SIZE];
  SNPRINTF(query, QUERY_SIZE,
           "CREATE TABLE IF NOT EXISTS %s(key BLOB PRIMARY KEY,value BLOB);",
           m_tableName);
  sqlite3_stmt* stmt;

  // prepare statement
  int retCode;
  retCode = sqlite3_prepare_v2(m_dbHandle, query, -1, &stmt, nullptr);

  // execute statement
  if (retCode == SQLITE_OK) retCode = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return retCode == SQLITE_DONE ? 0 : retCode;
}

int SqLiteHelper::insertKeyValue(void* keyData, int keyDataSize,
                                 void* valueData, int valueDataSize) {
  // construct query
  char query[QUERY_SIZE];
  SNPRINTF(query, QUERY_SIZE, "REPLACE INTO %s VALUES(?,?);", m_tableName);

  // prepare statement
  sqlite3_stmt* stmt;
  int retCode = sqlite3_prepare_v2(m_dbHandle, query, -1, &stmt, nullptr);
  if (retCode == SQLITE_OK) {
    // bind parameters and execte statement
    sqlite3_bind_blob(stmt, 1, keyData, keyDataSize, nullptr);
    sqlite3_bind_blob(stmt, 2, valueData, valueDataSize, nullptr);
    retCode = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return retCode == SQLITE_DONE ? 0 : retCode;
}

int SqLiteHelper::removeKey(void* keyData, int keyDataSize) {
  // construct query
  char query[QUERY_SIZE];
  SNPRINTF(query, QUERY_SIZE, "DELETE FROM %s WHERE key=?;", m_tableName);

  // prepare statement
  sqlite3_stmt* stmt;
  int retCode = sqlite3_prepare_v2(m_dbHandle, query, -1, &stmt, nullptr);
  if (retCode == SQLITE_OK) {
    // bind parameters and execte statement
    sqlite3_bind_blob(stmt, 1, keyData, keyDataSize, nullptr);
    retCode = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return retCode == SQLITE_DONE ? 0 : retCode;
}

int SqLiteHelper::getValue(void* keyData, int keyDataSize, void*& valueData,
                           int& valueDataSize) {
  // construct query
  char query[QUERY_SIZE];
  SNPRINTF(query, QUERY_SIZE,
           "SELECT value, length(value) AS valLength FROM %s WHERE key=?;",
           m_tableName);

  // prepare statement
  sqlite3_stmt* stmt;
  int retCode = sqlite3_prepare_v2(m_dbHandle, query, -1, &stmt, nullptr);
  if (retCode == SQLITE_OK) {
    // bind parameters and execte statement
    sqlite3_bind_blob(stmt, 1, keyData, keyDataSize, nullptr);
    retCode = sqlite3_step(stmt);
    if (retCode == SQLITE_ROW)  // we will get only one row
    {
      void* tempBuff = const_cast<void*>(sqlite3_column_blob(stmt, 0));
      valueDataSize = sqlite3_column_int(stmt, 1);
      valueData =
          reinterpret_cast<uint8_t*>(malloc(sizeof(uint8_t) * valueDataSize));
      memcpy(valueData, tempBuff, valueDataSize);
      retCode = sqlite3_step(stmt);
    }
  }

  sqlite3_finalize(stmt);
  return retCode == SQLITE_DONE ? 0 : retCode;
}

int SqLiteHelper::dropTable() {
  // create query
  char query[QUERY_SIZE];
  SNPRINTF(query, QUERY_SIZE, "DROP TABLE %s;", m_tableName);

  // prepare statement
  sqlite3_stmt* stmt;
  int retCode;
  retCode = sqlite3_prepare_v2(m_dbHandle, query, -1, &stmt, nullptr);

  // execute statement
  if (retCode == SQLITE_OK) retCode = sqlite3_step(stmt);

  sqlite3_finalize(stmt);
  return retCode == SQLITE_DONE ? 0 : retCode;
}

int SqLiteHelper::closeDB() {
  int retCode = dropTable();
  if (retCode == SQLITE_OK) retCode = sqlite3_close(m_dbHandle);

  return retCode;
}

int SqLiteHelper::executePragma(const char* pragmaName, int pragmaValue) {
  // create query
  char query[QUERY_SIZE];
  char strVal[50];
  SNPRINTF(strVal, 50, "%d", pragmaValue);
  SNPRINTF(query, QUERY_SIZE, "PRAGMA %s = %s;", pragmaName, strVal);

  // prepare statement
  sqlite3_stmt* stmt;
  int retCode;
  retCode = sqlite3_prepare_v2(m_dbHandle, query, -1, &stmt, nullptr);

  // execute PRAGMA
  if (retCode == SQLITE_OK &&
      sqlite3_step(stmt) == SQLITE_ROW) {  // PRAGMA command return one row
    retCode = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return retCode == SQLITE_DONE ? 0 : retCode;
}
