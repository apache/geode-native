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

#include "TcpSslConn.hpp"

#include <chrono>
#include <thread>

#include <geode/ExceptionTypes.hpp>
#include <geode/SystemProperties.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

const std::string TcpSslConn::kLibraryName = "cryptoImpl";

Ssl* TcpSslConn::getSSLImpl(ACE_HANDLE sock, const char* pubkeyfile,
                            const char* privkeyfile) {
  if (dll_.open(kLibraryName.c_str(), RTLD_NOW | RTLD_GLOBAL, 0) == -1) {
    auto msg = "Cannot open library: " + kLibraryName;
    LOGERROR(msg);
    throw FileNotFoundException(msg);
  }

  gf_create_SslImpl func =
      reinterpret_cast<gf_create_SslImpl>(dll_.symbol("gf_create_SslImpl"));
  if (func == nullptr) {
    auto msg = "Cannot find function gf_create_SslImpl in library cryptoImpl";
    LOGERROR(msg);
    throw IllegalStateException(msg);
  }

  return reinterpret_cast<Ssl*>(
      func(sock, pubkeyfile, privkeyfile, password_.c_str()));
}

void TcpSslConn::createSocket(ACE_HANDLE sock) {
  LOGDEBUG("Creating SSL socket stream");
  try {
    ssl_ = std::unique_ptr<Ssl>(
        getSSLImpl(sock, publicKeyFile_.c_str(), privateKeyFile_.c_str()));
  } catch (std::exception& e) {
    throw SslException(e.what());
  }
}

void TcpSslConn::connect() {
  using apache::geode::internal::chrono::duration::to_string;

  ACE_OS::signal(SIGPIPE, SIG_IGN);  // Ignore broken pipe

  LOGFINER(std::string("Connecting SSL socket stream to ") +
           inetAddress_.get_host_name() + ":" +
           std::to_string(inetAddress_.get_port_number()) + " waiting " +
           to_string(timeout_));

  if (ssl_->connect(inetAddress_, timeout_) == -1) {
    const auto lastError = ACE_OS::last_error();
    if (lastError == ETIME || lastError == ETIMEDOUT) {
      throw TimeoutException(
          "TcpSslConn::connect Attempt to connect timed out after " +
          to_string(timeout_) + ".");
    }
    close();
    throw GeodeIOException("TcpSslConn::connect failed with errno: " +
                           ACE_errno_to_string(lastError));
  }
}

void TcpSslConn::close() {
  if (ssl_) {
    ssl_->close();
    gf_destroy_SslImpl func =
        reinterpret_cast<gf_destroy_SslImpl>(dll_.symbol("gf_destroy_SslImpl"));
    func(ssl_.release());
  }
}

size_t TcpSslConn::socketOp(TcpConn::SockOp op, char* buff, size_t len,
                            std::chrono::microseconds waitDuration) {
  {
    // passing wait time as micro seconds
    ACE_Time_Value waitTime(waitDuration);
    auto endTime = std::chrono::steady_clock::now() + waitDuration;
    size_t readLen = 0;
    bool errnoSet = false;

    auto sendlen = len;
    size_t totalsend = 0;

    while (len > 0 && waitTime > ACE_Time_Value::zero) {
      if (len > kChunkSize) {
        sendlen = kChunkSize;
        len -= kChunkSize;
      } else {
        sendlen = len;
        len = 0;
      }
      do {
        ssize_t retVal;
        if (op == SOCK_READ) {
          retVal = ssl_->recv(buff, sendlen, &waitTime, &readLen);
        } else {
          retVal = ssl_->send(buff, sendlen, &waitTime, &readLen);
        }
        sendlen -= readLen;
        totalsend += readLen;
        if (retVal < 0) {
          int32_t lastError = ACE_OS::last_error();
          if (lastError == EAGAIN) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
          } else {
            errnoSet = true;
            break;
          }
        } else if (retVal == 0 && readLen == 0) {
          ACE_OS::last_error(EPIPE);
          errnoSet = true;
          break;
        }

        buff += readLen;

        waitTime = endTime - std::chrono::steady_clock::now();
        if (waitTime <= ACE_Time_Value::zero) break;
      } while (sendlen > 0);
      if (errnoSet) break;
    }

    if (len > 0 && !errnoSet) {
      ACE_OS::last_error(ETIME);
    }

    return totalsend;
  }
}

uint16_t TcpSslConn::getPort() {
  ACE_INET_Addr localAddr;
  ssl_->getLocalAddr(localAddr);
  return localAddr.get_port_number();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
