#pragma once

#ifndef GEODE_FUNCTIONSERVICEIMPL_H_
#define GEODE_FUNCTIONSERVICEIMPL_H_

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

#include <geode/internal/geode_globals.hpp>
#include <geode/AuthenticatedView.hpp>
#include <geode/FunctionService.hpp>
/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {
/**
 * @class FunctionService FunctionService.hpp
 * entry point for function execution
 * @see Execution
 */

class APACHE_GEODE_EXPORT FunctionServiceImpl : public FunctionService {
 public:
  explicit FunctionServiceImpl(AuthenticatedView* authenticatedView);

  virtual ~FunctionServiceImpl() {}

 private:
  explicit FunctionServiceImpl(const FunctionService&);
  FunctionServiceImpl& operator=(const FunctionService&);

  static std::shared_ptr<FunctionService> getFunctionService(
      AuthenticatedView* authenticatedView);

  AuthenticatedView* m_authenticatedView;

  friend class AuthenticatedView;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FUNCTIONSERVICEIMPL_H_
