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

#include "TcpConn.hpp"

#include <iomanip>
#include <iostream>

#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "util/Log.hpp"

namespace {
template <int Level, int Name>
class timeval {
 public:
  // This is not an instance of the template, but of the system provided type
  // to be written to the socket API.
#if defined(_WINDOWS)
  using value_type = DWORD;
#else
  using value_type = ::timeval;
#endif

 private:
  value_type value_{};

 public:
  timeval() {}

  explicit timeval(value_type v) : value_(v) {}

  timeval &operator=(value_type v) {
    value_ = v;
    return *this;
  }

  value_type value() const { return value_; }

  template <typename Protocol>
  int level(const Protocol &) const {
    return Level;
  }

  template <typename Protocol>
  int name(const Protocol &) const {
    return Name;
  }

  template <typename Protocol>
  value_type *data(const Protocol &) {
    return &value_;
  }

  template <typename Protocol>
  const value_type *data(const Protocol &) const {
    return &value_;
  }

  template <typename Protocol>
  std::size_t size(const Protocol &) const {
    return sizeof(value_);
  }

  template <typename Protocol>
  void resize(const Protocol &, std::size_t s) {
    if (s != sizeof(value_)) {
      throw std::length_error("timeval socket option resize");
    }
  }
};

// Asio doesn't support these socket options directly, but every major platform
// does. Timeout on IO socket operations are supported by the platform directly.
// This means We can all receive without needing to use the timeout interface -
// and more importantly, we can send while holding to per-operation time
// constraints and without blocking indefinitely.
//
// The default timeout is infinite, or by setting the socket option to null,
// which I won't provide - just don't construct a TcpConn with send and
// receieve timeouts.
typedef timeval<SOL_SOCKET, SO_SNDTIMEO> send_timeout;
typedef timeval<SOL_SOCKET, SO_RCVTIMEO> receive_timeout;
}  // namespace

namespace apache {
namespace geode {
namespace client {
TcpConn::TcpConn(const std::string ipaddr,
                 std::chrono::microseconds connect_timeout,
                 int32_t maxBuffSizePool)
    : TcpConn{
          ipaddr.substr(0, ipaddr.find(':')),
          static_cast<uint16_t>(std::stoi(ipaddr.substr(ipaddr.find(':') + 1))),
          connect_timeout, maxBuffSizePool} {}

TcpConn::TcpConn(const std::string host, uint16_t port,
                 std::chrono::microseconds timeout, int32_t maxBuffSizePool)
    : socket_{io_context_} {
  auto results = resolve(host, port);

  // We must connect first so we have a valid file descriptor to set options
  // on.
  connect(results, timeout);

  socket_.set_option(::boost::asio::ip::tcp::no_delay{true});
  socket_.set_option(
      ::boost::asio::socket_base::send_buffer_size{maxBuffSizePool});
  socket_.set_option(
      ::boost::asio::socket_base::receive_buffer_size{maxBuffSizePool});
}

TcpConn::TcpConn(const std::string ipaddr,
                 std::chrono::microseconds connect_timeout,
                 int32_t maxBuffSizePool, std::chrono::microseconds send_time,
                 std::chrono::microseconds receive_time)
    : TcpConn{ipaddr, connect_timeout, maxBuffSizePool} {
#if defined(_WINDOWS)
  socket_.set_option(::send_timeout{static_cast<DWORD>(send_time.count())});
  socket_.set_option(
      ::receive_timeout{static_cast<DWORD>(receive_time.count())});
#else

  auto send_seconds =
      std::chrono::duration_cast<std::chrono::seconds>(send_time);
  auto send_microseconds =
      send_time % std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::seconds{1});
  socket_.set_option(
      ::send_timeout{{static_cast<int>(send_seconds.count()),
                      static_cast<int>(send_microseconds.count())}});

  auto receive_seconds =
      std::chrono::duration_cast<std::chrono::seconds>(receive_time);
  auto receive_microseconds =
      receive_time % std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::seconds{1});
  socket_.set_option(
      ::receive_timeout{{static_cast<int>(receive_seconds.count()),
                         static_cast<int>(receive_microseconds.count())}});
#endif
}

TcpConn::~TcpConn() {
  try {
    LOG_FINE("Disconnected %s:%u -> %s:%u",
             socket_.local_endpoint().address().to_string().c_str(),
             socket_.local_endpoint().port(),
             socket_.remote_endpoint().address().to_string().c_str(),
             socket_.remote_endpoint().port());
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

  } catch (...) {
    LOG_FINE("Closed socket %s:%u ",
             socket_.local_endpoint().address().to_string().c_str(),
             socket_.local_endpoint().port());
  }

  socket_.close();
}

size_t TcpConn::receive(char *buff, const size_t len,
                        std::chrono::milliseconds timeout) {
  LOG_DEBUG("Receiving %d bytes from %s:%u -> %s:%u", len,
            socket_.remote_endpoint().address().to_string().c_str(),
            socket_.remote_endpoint().port(),
            socket_.local_endpoint().address().to_string().c_str(),
            socket_.local_endpoint().port());
  return receive(buff, len, timeout, true);
}

size_t TcpConn::receive_nothrowiftimeout(char *buff, const size_t len,
                                         std::chrono::milliseconds timeout) {
  LOG_DEBUG("Receiving an unknown number of bytes from %s:%d -> %s:%d",
            socket_.remote_endpoint().address().to_string().c_str(),
            socket_.remote_endpoint().port(),
            socket_.local_endpoint().address().to_string().c_str(),
            socket_.local_endpoint().port());
  return receive(buff, len, timeout, false);
}

size_t TcpConn::receive(char *buff, const size_t len,
                        std::chrono::milliseconds timeout,
                        bool throwTimeoutException) {
  boost::optional<boost::system::error_code> read_result;
  std::size_t bytes_read = 0;

  auto beforeReadPoint = std::chrono::system_clock::now();

  try {
    prepareAsyncRead(buff, len, read_result, bytes_read);
    io_context_.restart();
    io_context_.run_for(timeout);
  } catch (...) {
    LOG_DEBUG("Throwing an unexpected read exception");
    throw;
  }

  if (read_result && *read_result) {
    LOG_DEBUG("Throwing a read exception: %s", read_result->message().c_str());
    socket_.cancel();
    // Get the abort
    io_context_.restart();
    io_context_.run();
    throw boost::system::system_error{*read_result};
  }

  if (bytes_read == 0) {
    auto elapsedTime = std::chrono::duration<double, std::micro>(
        std::chrono::system_clock::now() - beforeReadPoint);
    if (elapsedTime < timeout) {
      LOG_DEBUG("Throwing an IO exception");
      socket_.cancel();
      // Get the abort
      io_context_.restart();
      io_context_.run();
      throw boost::system::system_error{boost::asio::error::broken_pipe};
    } else {
      LOG_DEBUG("Throwing an eof exception");
      socket_.cancel();
      // Get the abort
      io_context_.restart();
      io_context_.run();
      throw boost::system::system_error{boost::asio::error::eof};
    }
  }

  if (bytes_read != len && throwTimeoutException) {
    LOG_DEBUG("Throwing a read timeout exception");
    socket_.cancel();
    // Get the abort
    io_context_.restart();
    io_context_.run();
    throw boost::system::system_error{boost::asio::error::operation_aborted};
  }

  return bytes_read;
}

size_t TcpConn::send(const char *buff, const size_t len,
                     std::chrono::milliseconds timeout) {
  LOG_DEBUG("Sending %d bytes from %s:%u -> %s:%u", len,
            socket_.local_endpoint().address().to_string().c_str(),
            socket_.local_endpoint().port(),
            socket_.remote_endpoint().address().to_string().c_str(),
            socket_.remote_endpoint().port());

  boost::optional<boost::system::error_code> write_result;
  std::size_t bytes_written = 0;

  try {
    prepareAsyncWrite(buff, len, write_result, bytes_written);
    io_context_.restart();
    io_context_.run_for(timeout);
  } catch (...) {
    LOG_DEBUG("Throwing an unexpected write exception");
    throw;
  }

  if (write_result && *write_result) {
    LOG_DEBUG("Throwing a write exception. %s",
              write_result->message().c_str());
    throw boost::system::system_error{*write_result};
  }

  if (bytes_written != len) {
    LOG_DEBUG("Throwing a write timeout exception");
    socket_.cancel();
    // Get the abort
    io_context_.restart();
    io_context_.run();
    throw boost::system::system_error{boost::asio::error::operation_aborted};
  }

  return bytes_written;
}

//  Return the local port for this TCP connection.
uint16_t TcpConn::getPort() { return socket_.local_endpoint().port(); }

void TcpConn::connect(boost::asio::ip::tcp::resolver::results_type r,
                      std::chrono::microseconds timeout) {
  boost::optional<boost::system::error_code> connect_result;

  try {
    // We must connect first so we have a valid file descriptor to set
    // options on.
    boost::asio::async_connect(
        socket_, r,
        [&connect_result](const boost::system::error_code &ec,
                          const boost::asio::ip::tcp::endpoint) {
          connect_result = ec;
        });

    io_context_.restart();
    io_context_.run_for(timeout);
  } catch (...) {
    LOG_DEBUG("Throwing an unexpected connect exception");
    throw;
  }

  if (connect_result && *connect_result) {
    LOG_DEBUG("Throwing a connect exception: %s",
              connect_result->message().c_str());
    throw boost::system::system_error{*connect_result};
  }

  if (!connect_result) {
    LOG_DEBUG("Throwing a connect timeout exception");
    throw boost::system::system_error{boost::asio::error::operation_aborted};
  }

  LOG_DEBUG("Connected %s:%u -> %s:%u",
            socket_.local_endpoint().address().to_string().c_str(),
            socket_.local_endpoint().port(),
            socket_.remote_endpoint().address().to_string().c_str(),
            socket_.remote_endpoint().port());
}

boost::asio::ip::tcp::resolver::results_type TcpConn::resolve(
    const std::string host, uint16_t port) {
  boost::system::error_code resolve_result;
  boost::asio::ip::tcp::resolver::results_type results;

  boost::asio::ip::tcp::resolver resolver(io_context_);
  results = resolver.resolve(host, std::to_string(port), resolve_result);

  if (resolve_result) {
    LOG_DEBUG("Throwing a resolve exception: %s",
              resolve_result.message().c_str());
    throw boost::system::system_error{resolve_result};
  }

  return results;
}

void TcpConn::prepareAsyncRead(
    char *buff, size_t len,
    boost::optional<boost::system::error_code> &read_result,
    std::size_t &bytes_read) {
  boost::asio::async_read(
      socket_, boost::asio::buffer(buff, len),
      [&read_result, &bytes_read](const boost::system::error_code &ec,
                                  const size_t n) {
        bytes_read = n;

        // EOF itself occurs when there is no data available on the socket at
        // the time of the read. It may simply imply data has yet to arrive.
        // Do nothing. Defer to timeout rather than assume a broken
        // connection.
        if (ec != boost::asio::error::eof &&
            ec != boost::asio::error::try_again) {
          read_result = ec;
          return;
        }
      });
}

void TcpConn::prepareAsyncWrite(
    const char *buff, size_t len,
    boost::optional<boost::system::error_code> &write_result,
    std::size_t &bytes_written) {
  boost::asio::async_write(
      socket_, boost::asio::buffer(buff, len),
      [&write_result, &bytes_written](const boost::system::error_code &ec,
                                      const size_t n) {
        bytes_written = n;

        if (ec != boost::asio::error::eof &&
            ec != boost::asio::error::try_again) {
          write_result = ec;
          return;
        }
      });
}

}  // namespace client
}  // namespace geode
}  // namespace apache
