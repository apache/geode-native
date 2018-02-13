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

#include "DiffieHellman.hpp"
#include "util/Log.hpp"
#include <geode/ExceptionTypes.hpp>
#include <geode/SystemProperties.hpp>
#include <ace/Guard_T.h>
namespace apache {
namespace geode {
namespace client {

ACE_DLL DiffieHellman::m_dll;

#define INIT_DH_FUNC_PTR(OrigName) \
  DiffieHellman::OrigName##_Type DiffieHellman::OrigName##_Ptr = nullptr;

INIT_DH_FUNC_PTR(gf_initDhKeys)
INIT_DH_FUNC_PTR(gf_clearDhKeys)
INIT_DH_FUNC_PTR(gf_getPublicKey)
INIT_DH_FUNC_PTR(gf_setPublicKeyOther)
INIT_DH_FUNC_PTR(gf_computeSharedSecret)
INIT_DH_FUNC_PTR(gf_encryptDH)
INIT_DH_FUNC_PTR(gf_decryptDH)
INIT_DH_FUNC_PTR(gf_verifyDH)

void* DiffieHellman::getOpenSSLFuncPtr(const char* function_name) {
  void* func = m_dll.symbol(function_name);
  if (func == nullptr) {
    char msg[1000];
    ACE_OS::snprintf(msg, 1000, "cannot find function %s in library %s",
                     function_name, "cryptoImpl");
    LOGERROR(msg);
    throw IllegalStateException(msg);
  }
  return func;
}

void DiffieHellman::initOpenSSLFuncPtrs() {
  static bool inited = false;

  if (inited) {
    return;
  }

  const char* libName = "cryptoImpl";

  if (m_dll.open(libName, ACE_DEFAULT_SHLIB_MODE, 0) == -1) {
    char msg[1000];
    ACE_OS::snprintf(msg, 1000, "cannot open library: %s", libName);
    LOGERROR(msg);
    throw FileNotFoundException(msg);
  }

#define ASSIGN_DH_FUNC_PTR(OrigName) \
  OrigName##_Ptr = (OrigName##_Type)getOpenSSLFuncPtr(#OrigName);

  ASSIGN_DH_FUNC_PTR(gf_initDhKeys)
  ASSIGN_DH_FUNC_PTR(gf_clearDhKeys)
  ASSIGN_DH_FUNC_PTR(gf_getPublicKey)
  ASSIGN_DH_FUNC_PTR(gf_setPublicKeyOther)
  ASSIGN_DH_FUNC_PTR(gf_computeSharedSecret)
  ASSIGN_DH_FUNC_PTR(gf_encryptDH)
  ASSIGN_DH_FUNC_PTR(gf_decryptDH)
  ASSIGN_DH_FUNC_PTR(gf_verifyDH)

  inited = true;
}

void DiffieHellman::initDhKeys(const std::shared_ptr<Properties>& props) {
  m_dhCtx = nullptr;

  const auto& dhAlgo = props->find(SecurityClientDhAlgo);
  const auto& ksPath = props->find(SecurityClientKsPath);

  // Null check only for DH Algo
  if (dhAlgo == nullptr) {
    LOGFINE("DH algo not available");
    return;
  }

  int error =
      gf_initDhKeys_Ptr(&m_dhCtx, dhAlgo->value().c_str(),
                        ksPath != nullptr ? ksPath->value().c_str() : nullptr);

  if (error == DH_ERR_UNSUPPORTED_ALGO) {  // Unsupported Algorithm
    char msg[64] = {'\0'};
    ACE_OS::snprintf(msg, 64, "Algorithm %s is not supported.",
                     dhAlgo->value().c_str());
    throw IllegalArgumentException(msg);
  } else if (error == DH_ERR_ILLEGAL_KEYSIZE) {  // Illegal Key size
    char msg[64] = {'\0'};
    ACE_OS::snprintf(msg, 64, "Illegal key size for algorithm %s.",
                     dhAlgo->value().c_str());
    throw IllegalArgumentException(msg);
  } else if (m_dhCtx == nullptr) {
    throw IllegalStateException(
        "Could not initialize the Diffie-Hellman helper");
  }
}

void DiffieHellman::clearDhKeys(void) {
  // Sanity check for accidental calls
  if (gf_clearDhKeys_Ptr == nullptr) {
    return;
  }

  gf_clearDhKeys_Ptr(m_dhCtx);

  m_dhCtx = nullptr;

  return;
}
std::shared_ptr<CacheableBytes> DiffieHellman::getPublicKey(void) {
  int keyLen = 0;
  auto pubKeyPtr = gf_getPublicKey_Ptr(m_dhCtx, &keyLen);
  return CacheableBytes::create(std::vector<int8_t>(pubKeyPtr, pubKeyPtr +
                                      keyLen));
}

void DiffieHellman::setPublicKeyOther(
    const std::shared_ptr<CacheableBytes>& pubkey) {
  return gf_setPublicKeyOther_Ptr(
      m_dhCtx, reinterpret_cast<const uint8_t*>(pubkey->value().data()),
      pubkey->length());
}

void DiffieHellman::computeSharedSecret(void) {
  return gf_computeSharedSecret_Ptr(m_dhCtx);
}
std::shared_ptr<CacheableBytes> DiffieHellman::encrypt(
    const std::shared_ptr<CacheableBytes>& cleartext) {
  return encrypt(reinterpret_cast<const uint8_t*>(cleartext->value().data()),
                 cleartext->length());
}
std::shared_ptr<CacheableBytes> DiffieHellman::encrypt(const uint8_t* cleartext,
                                                       int len) {
  int cipherLen = 0;
  unsigned char* ciphertextPtr =
      gf_encryptDH_Ptr(m_dhCtx, cleartext, len, &cipherLen);
  return CacheableBytes::create(std::vector<int8_t>(ciphertextPtr, ciphertextPtr +
                                      cipherLen));
}
std::shared_ptr<CacheableBytes> DiffieHellman::decrypt(
    const std::shared_ptr<CacheableBytes>& cleartext) {
  return decrypt(reinterpret_cast<const uint8_t*>(cleartext->value().data()),
                 cleartext->length());
}
std::shared_ptr<CacheableBytes> DiffieHellman::decrypt(const uint8_t* cleartext,
                                                       int len) {
  int cipherLen = 0;
  unsigned char* ciphertextPtr =
      gf_decryptDH_Ptr(m_dhCtx, cleartext, len, &cipherLen);
  return CacheableBytes::create(std::vector<int8_t>(ciphertextPtr, ciphertextPtr +
                                      cipherLen));
}

bool DiffieHellman::verify(const std::shared_ptr<CacheableString>& subject,
                           const std::shared_ptr<CacheableBytes>& challenge,
                           const std::shared_ptr<CacheableBytes>& response) {
  int errCode = DH_ERR_NO_ERROR;
  LOGDEBUG("DiffieHellman::verify");
  bool result = gf_verifyDH_Ptr(
      m_dhCtx, subject->value().c_str(),
      reinterpret_cast<const uint8_t*>(challenge->value().data()), challenge->length(),
      reinterpret_cast<const uint8_t*>(response->value().data()), response->length(),
      &errCode);
  LOGDEBUG("DiffieHellman::verify 2");
  if (errCode == DH_ERR_SUBJECT_NOT_FOUND) {
    LOGERROR("Subject name %s not found in imported certificates.",
             subject->value().c_str());
  } else if (errCode == DH_ERR_NO_CERTIFICATES) {
    LOGERROR("No imported certificates.");
  } else if (errCode == DH_ERR_INVALID_SIGN) {
    LOGERROR("Signature varification failed.");
  }

  return result;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
