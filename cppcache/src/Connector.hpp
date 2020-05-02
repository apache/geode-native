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

#ifndef GEODE_CONNECTOR_H_
#define GEODE_CONNECTOR_H_

#include <chrono>

#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

constexpr std::chrono::microseconds DEFAULT_CONNECT_TIMEOUT =
    std::chrono::seconds(10);

constexpr std::chrono::milliseconds DEFAULT_TIMEOUT = std::chrono::seconds(15);

constexpr std::chrono::milliseconds DEFAULT_READ_TIMEOUT = DEFAULT_TIMEOUT;

constexpr std::chrono::milliseconds DEFAULT_WRITE_TIMEOUT = DEFAULT_TIMEOUT;

class Connector {
 public:
  Connector() {}
  virtual ~Connector() {}
  Connector(const Connector &) = delete;
  Connector &operator=(const Connector &) = delete;

  /**
   * Reads <code>len</code> bytes of data and stores them into the buffer
   * array <code>b</code>. The number of bytes actually read is returned as an
   * integer.  This method blocks until <code>len</code> bytes of data is
   * read, or an exception is thrown.
   *
   * <p> If <code>b</code> is <code>null</code>, or <code>len</code> is
   * less than or equal to <code>0</code> an
   * <code>IllegalArgumentException</code>
   * is thrown.
   *
   * <p> If <code>len</code> bytes cannot be read for any reason, then an
   * <code>GeodeIOException</code> is thrown.
   *
   * <p> The <code>read(b)</code> method for class <code>InputStream</code>
   * has the same effect as: <pre><code> read(b, 0, b.length) </code></pre>
   *
   * @param      b   the buffer into which the data is read.
   * @param      len   the number of bytes to read.
   * @param      waitSeconds   the number of seconds to allow the read to
   * complete.
   * @return     the total number of bytes read into the buffer, or
   *             <code>-1</code> if an error was encountered.
   * @exception  GeodeIOException, TimeoutException, IllegalArgumentException,
   * OutOfMemoryException.
   */
  virtual size_t receive(char *b, size_t len,
                         std::chrono::microseconds waitSeconds) = 0;

  /**
   * Writes <code>len</code> bytes from the specified byte array
   * to the underlying output stream.
   *
   * @param      b     the data.
   * @param      len   the number of bytes to write.
   * @param      waitSeconds   the number of seconds to allow the write to
   * complete.
   * @return     the actual number of bytes written.
   * @exception  GeodeIOException, TimeoutException, IllegalArgumentException.
   */
  virtual size_t send(const char *b, size_t len,
                      std::chrono::microseconds waitSeconds) = 0;

  /**
   * Initialises the connection.
   */
  virtual void init() = 0;

  /**
   * Closes the connection.
   */
  virtual void close() = 0;

  /**
   * Returns local port for this TCP connection
   */
  virtual uint16_t getPort() = 0;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CONNECTOR_H_
