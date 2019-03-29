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

#ifndef OPENSSL_COMPAT_H
#define OPENSSL_COMPAT_H

#include <openssl/opensslv.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L

#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

static inline void DH_get0_pqg(const DH *dh, const BIGNUM **p, const BIGNUM **q,
                               const BIGNUM **g) {
  if (p) *p = dh->p;
  if (q) *q = dh->q;
  if (g) *g = dh->g;
}

static inline void DH_get0_key(const DH *dh, const BIGNUM **pub_key,
                               const BIGNUM **priv_key) {
  if (pub_key) *pub_key = dh->pub_key;
  if (priv_key) *priv_key = dh->priv_key;
}

static inline int DH_set0_key(DH *dh, BIGNUM *pub_key, BIGNUM *priv_key) {
  if (!(dh->pub_key || pub_key)) {
    return 0;
  }

  if (pub_key) {
    BN_free(dh->pub_key);
    dh->pub_key = pub_key;
  }
  if (priv_key) {
    BN_free(dh->priv_key);
    dh->priv_key = priv_key;
  }

  return 1;
}

static inline int DH_set_length(DH *dh, long length) {
  dh->length = length;
  return 1;
}

static inline EVP_MD_CTX *EVP_MD_CTX_new(void) {
  return reinterpret_cast<EVP_MD_CTX *>(OPENSSL_malloc(sizeof(EVP_MD_CTX)));
}

static inline void EVP_MD_CTX_free(EVP_MD_CTX *ctx) {
  EVP_MD_CTX_cleanup(ctx);
  OPENSSL_free(ctx);
}

static inline int EVP_PKEY_up_ref(EVP_PKEY *pkey) {
  return CRYPTO_add(&pkey->references, 1, CRYPTO_LOCK_EVP_PKEY);
}

static inline void X509_ALGOR_get0(const ASN1_OBJECT **paobj, int *,
                                   const void **, const X509_ALGOR *algor) {
  *paobj = algor->algorithm;
}

#define X509_F_X509_PUBKEY_DECODE X509_F_X509_PUBKEY_GET

#endif /* OPENSSL_VERSION_NUMBER */

#endif /* OPENSSL_COMPAT_H */
