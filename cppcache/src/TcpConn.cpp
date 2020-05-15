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
    if (s != sizeof(value_))
      throw std::length_error("timeval socket option resize");
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
  boost::optional<boost::system::error_code> connect_result, timer_result;
  boost::asio::deadline_timer connect_deadline{io_context_};

  try {
    // We must connect first so we have a valid file descriptor to set options
    // on.
    boost::asio::async_connect(
        socket_,
        boost::asio::ip::tcp::resolver(io_context_)
            .resolve(host, std::to_string(port)),
        [&connect_result](const boost::system::error_code &ec,
                          const boost::asio::ip::tcp::endpoint) -> bool {
          connect_result.reset(ec);
          return true;
        });

    connect_deadline.expires_from_now(
        boost::posix_time::milliseconds(timeout.count()));
    connect_deadline.async_wait(
        [&timer_result](const boost::system::error_code &ec) {
          if (ec) {
            timer_result.reset(ec);
          }
        });

    io_context_.reset();
    while (io_context_.run_one()) {
      if (timer_result) {
        socket_.cancel();
      }
      if (connect_result) {
        connect_deadline.cancel();
      }
    }
  } catch (...) {
    std::cout << "Throwing an unexpected connect exception\n";
    throw;
  }

  if (connect_result && *connect_result) {
    std::cout << "Throwing a connect exception\n";
    throw *connect_result;
  }

  std::stringstream ss;
  ss << "Connected " << socket_.local_endpoint() << " -> "
     << socket_.remote_endpoint();
  LOGINFO(ss.str());

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
  std::stringstream ss;

  try {
    ss << "Disconnected " << socket_.local_endpoint() << " -> "
       << socket_.remote_endpoint();

    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);

  } catch (...) {
    ss = std::stringstream{};

    ss << "Closed socket " << socket_.local_endpoint();
  }

  socket_.close();

  LOGFINE(ss.str());
}

size_t TcpConn::receive(char *buff, const size_t len,
                        std::chrono::milliseconds timeout) {
  std::stringstream ss;
  ss << "Receiving " << len << " bytes from " << socket_.remote_endpoint()
     << " -> " << socket_.local_endpoint();
  LOGDEBUG(ss.str());

  boost::optional<boost::system::error_code> timer_result, read_result;
  std::size_t bytes_read = 0;

  try {
    // Here we prep the Asio subsystem for a read operation with the completion
    // condition below.
    boost::asio::async_read(
        socket_, boost::asio::buffer(buff, len),
        [&read_result, &bytes_read, len](const boost::system::error_code &ec,
                                         const size_t n) -> size_t {
          bytes_read = n;

          // Aborts come from timeouts or manual interrupts, as seen below in
          // the while loop. If we timeout and haven't read anything, the
          // connection is probably broken. A broken pipe is indicated by an
          // EOF.
          if (ec == boost::asio::error::operation_aborted && 0 == n) {
            read_result.reset(
                boost::system::error_code{boost::asio::error::eof});
            return 0;
          }
          // If we timeout and there are bytes read, that isn't necessarily an
          // error; Asio presumes it's meant to fill a fixed size buffer
          // exactly. The buffer may simply be too big for an expected response
          // but of an unknown size.
          //
          // EOF itself occurs when there is no data available on the socket at
          // the time of the read. It may simply imply data has yet to arrive.
          // Do nothing. Defer to timeout rather than assume a broken
          // connection.
          //
          // For every other error condition, including a timeout with data,
          // complete the operation.
          else if (ec && ec != boost::asio::error::eof &&
                   ec != boost::asio::error::try_again) {
            read_result.reset(ec);
            return 0;
          }
          // Once the buffer is filled, indicate success, regardless the error
          // condition on the socket. Defer to the next receive operation to
          // handle that eventuality.
          else if (n == len) {
            read_result.reset(boost::system::error_code{});
            return 0;
          }

          // As the last read was successful, continue filling the fixed size
          // buffer.
          return len - n;
        });

    // This timer will abort the operation after the timeout period, and that
    // will be indicated within the completion handler above.
    boost::asio::deadline_timer read_deadline{io_context_};
    read_deadline.expires_from_now(
        boost::posix_time::milliseconds(timeout.count()));
    read_deadline.async_wait(
        [&timer_result](const boost::system::error_code &ec) {
          if (ec) {
            timer_result.reset(ec);
          }
        });

    // Run until the context enters the stopped state.
    io_context_.reset();
    while (io_context_.run_one()) {
      // If something went wrong with the timer, abort the read.
      // This will result in an aborted read result.
      if (timer_result) {
        socket_.cancel();
      }
      if (read_result) {
        read_deadline.cancel();
      }
    }
  } catch (...) {
    std::cout << "Throwing an unexpected read exception\n";
    throw;
  }

  if (read_result && *read_result) {
    std::cout << "Throwing a read exception\n";
    throw *read_result;
  }

  return bytes_read;
}

size_t TcpConn::send(const char *buff, const size_t len,
                     std::chrono::milliseconds timeout) {
  std::stringstream ss;
  ss << "Sending " << len << " bytes from " << socket_.local_endpoint()
     << " -> " << socket_.remote_endpoint();
  LOGDEBUG(ss.str());

  boost::optional<boost::system::error_code> timer_result, write_result;
  std::size_t bytes_written = 0;

  try {
    boost::asio::async_write(
        socket_, boost::asio::buffer(buff, len),
        [&write_result, &bytes_written, len](
            const boost::system::error_code &ec, const size_t n) -> size_t {
          bytes_written = n;

          if (ec == boost::asio::error::operation_aborted && 0 == n) {
            write_result.reset(
                boost::system::error_code{boost::asio::error::eof});
            return 0;
          } else if (ec && ec != boost::asio::error::eof &&
                     ec != boost::asio::error::try_again) {
            write_result.reset(ec);
            return 0;
          } else if (n == len) {
            write_result.reset(boost::system::error_code{});
            return 0;
          }

          return len - n;
        });

    boost::asio::deadline_timer write_deadline{io_context_};
    write_deadline.expires_from_now(
        boost::posix_time::milliseconds(timeout.count()));
    write_deadline.async_wait(
        [&timer_result](const boost::system::error_code &ec) {
          if (ec) {
            timer_result.reset(ec);
          }
        });

    io_context_.reset();
    while (io_context_.run_one()) {
      if (timer_result) {
        socket_.cancel();
      }
      if (write_result) {
        write_deadline.cancel();
      }
    }
  } catch (...) {
    std::cout << "Throwing an unexpected write exception\n";
    throw;
  }

  if (write_result && *write_result) {
    std::cout << "Throwing a write exception\n";
    throw *write_result;
  }

  return bytes_written;
}

//  Return the local port for this TCP connection.
uint16_t TcpConn::getPort() { return socket_.local_endpoint().port(); }

}  // namespace client
}  // namespace geode
}  // namespace apache
