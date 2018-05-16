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

#ifndef GEODE_PARTITIONRESOLVER_H_
#define GEODE_PARTITIONRESOLVER_H_

#include <iosfwd>
#include <memory>
#include <string>

#include "Serializable.hpp"
#include "internal/geode_base.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheableKey;
class EntryEvent;

/**
 * Implement the <code>PartitionResolver</code> interface to enable custom
 * partitioning on the <code>PartitionedRegion</code>.
 *
 * The Key class or callback arg can implement the PartitionResolver interface
 * to enable custom partitioning, or you can configure your own
 * PartitionResolver class in partition attributes (for instance when the Key is
 * a primitive type or String):
 * - Implement the appropriate equals. For all implementations, you need to be
 * sure to code the class equals method so it properly verifies equality for the
 * PartitionResolver implementation. This might mean verifying that class names
 * are the same or that the returned routing objects are the same etc. When you
 * initiate the partitioned region on multiple nodes, Geode uses the equals
 * method to ensure you are using the same PartitionResolver implementation for
 * all of the nodes for the region.
 * - Geode uses the routing object's hashCode to determine where the data is
 * being managed. Say, for example, you want to colocate all Trades by month and
 * year. The key is implemented by a TradeKey class which also implements the
 * PartitionResolver interface:
 *
 * @code
 * public class TradeKey implements PartitionResolver {
 *   private String tradeID;
 *   private Month month;
 *   private Year year;
 *
 *   public TradingKey() { }
 *   public TradingKey(Month month, Year year) {
 *     this.month = month;
 *     this.year = year;
 *   }
 *   public Serializable getRoutingObject(EntryOperation opDetails) {
 *     return this.month + this.year;
 *   }
 * }
 * @endcode
 *
 * In the example above, all trade entries with the same month and year are
 * guaranteed to be colocated.
 */
class APACHE_GEODE_EXPORT PartitionResolver {
  /**
   * @brief public methods
   */

 public:
  /**
   * @brief destructor
   */
  virtual ~PartitionResolver();

  /**
   * Returns the name of the PartitionResolver
   * @return String name
   */
  virtual const std::string& getName();

  /**
   * @param opDetails the detail of the entry event
   * @throws RuntimeException - any exception thrown will terminate the
   * operation and the exception will be passed to the calling thread.
   * @return object associated with entry event which allows the Partitioned
   * Region to store associated data together
   */
  virtual std::shared_ptr<CacheableKey> getRoutingObject(
      const EntryEvent& opDetails) = 0;

 protected:
  /**
   * @brief constructors
   */
  PartitionResolver();

 private:
  PartitionResolver(const PartitionResolver& other) = delete;
  void operator=(const PartitionResolver& other) = delete;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PARTITIONRESOLVER_H_
