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

/*
 * The RemoteQuery QuickStart Example.
 *
 * This example takes the following steps:
 *
 * 1. Create a Geode Cache Programmatically.
 * 2. Create the example Region Programmatically.
 * 3. Populate some query objects on the Region.
 * 4. Execute a query that returns a Result Set.
 * 5. Execute a query that returns a Struct Set.
 * 6. Execute the region shortcut/convenience query methods.
 * 7. Close the Cache.
 *
 */

// Include the Geode library.
#include <geode/GeodeCppCache.hpp>

// Include our Query objects, viz. Portfolio and Position.
#include "queryobjects/Portfolio.hpp"
#include "queryobjects/Position.hpp"

// Use the "geode" namespace.
using namespace apache::geode::client;

// Use the "testobject" namespace for the query objects.
using namespace testobject;

// The RemoteQuery QuickStart example.
int main(int argc, char** argv) {
  try {
    // Create a Geode Cache Programmatically.
    auto cacheFactory = CacheFactory::createCacheFactory();
    auto cachePtr = cacheFactory->setSubscriptionEnabled(true)->create();

    LOGINFO("Created the Geode Cache");

    auto regionFactory = cachePtr->createRegionFactory(CACHING_PROXY);

    LOGINFO("Created the RegionFactory");

    // Create the example Region programmatically.
    auto regionPtr = regionFactory->create("Portfolios");

    LOGINFO("Created the Region.");

    // Register our Serializable/Cacheable Query objects, viz. Portfolio and
    // Position.
    serializationRegistry->addType(Portfolio::createDeserializable);
    serializationRegistry->addType(Position::createDeserializable);

    LOGINFO("Registered Serializable Query Objects");

    // Populate the Region with some Portfolio objects.
    std::shared_ptr<Portfolio> port1Ptr(new Portfolio(1 /*ID*/, 10 /*size*/));
    std::shared_ptr<Portfolio> port2Ptr(new Portfolio(2 /*ID*/, 20 /*size*/));
    std::shared_ptr<Portfolio> port3Ptr(new Portfolio(3 /*ID*/, 30 /*size*/));
    regionPtr->put("Key1", port1Ptr);
    regionPtr->put("Key2", port2Ptr);
    regionPtr->put("Key3", port3Ptr);

    LOGINFO("Populated some Portfolio Objects");

    // Get the QueryService from the Cache.
    auto qrySvcPtr = cachePtr->getQueryService();

    LOGINFO("Got the QueryService from the Cache");

    // Execute a Query which returns a ResultSet.
    auto qryPtr = qrySvcPtr->newQuery("SELECT DISTINCT * FROM /Portfolios");
    std::shared_ptr<SelectResults> resultsPtr = qryPtr->execute();

    LOGINFO("ResultSet Query returned %d rows", resultsPtr->size());

    // Execute a Query which returns a StructSet.
    qryPtr = qrySvcPtr->newQuery(
        "SELECT DISTINCT ID, status FROM /Portfolios WHERE ID > 1");
    resultsPtr = qryPtr->execute();

    LOGINFO("StructSet Query returned %d rows", resultsPtr->size());

    // Iterate through the rows of the query result.
    int rowCount = 0;
    SelectResultsIterator iter = resultsPtr->getIterator();
    while (iter.hasNext()) {
      rowCount++;
      Struct* psi = dynamic_cast<Struct*>(iter.next().get());
      LOGINFO("Row %d Column 1 is named %s, value is %s", rowCount,
              psi->getFieldName(0).c_str(), (*psi)[0]->toString()->asChar());
      LOGINFO("Row %d Column 2 is named %s, value is %s", rowCount,
              psi->getFieldName(1).c_str(), (*psi)[1]->toString()->asChar());
    }

    // Execute a Region Shortcut Query (convenience method).
    resultsPtr = regionPtr->query("ID = 2");

    LOGINFO("Region Query returned %d rows", resultsPtr->size());

    // Execute the Region selectValue() API.
    std::shared_ptr<Serializable> resultPtr = regionPtr->selectValue("ID = 3");
    auto portPtr = std::dynamic_pointer_cast<Portfolio>(resultPtr);

    LOGINFO("Region selectValue() returned an item:\n %s",
            portPtr->toString()->asChar());

    // Execute the Region existsValue() API.
    bool existsValue = regionPtr->existsValue("ID = 4");

    LOGINFO("Region existsValue() returned %s", existsValue ? "true" : "false");

    // Execute the parameterized query
    // Populate the parameter list (paramList) for the query.
    auto pqryPtr = qrySvcPtr->newQuery(
        "SELECT DISTINCT ID, status FROM /Portfolios WHERE ID > $1 and "
        "status=$2");

    std::shared_ptr<CacheableVector> paramList = CacheableVector::create();
    paramList->push_back(Cacheable::create(1));         // Param-1
    paramList->push_back(Cacheable::create("active"));  // Param-2

    std::shared_ptr<SelectResults> pqresultsPtr = pqryPtr->execute(paramList);

    LOGINFO("StructSet Query returned %d rows", pqresultsPtr->size());

    // Iterate through the rows of the query result.
    rowCount = 0;
    SelectResultsIterator itr = pqresultsPtr->getIterator();
    while (itr.hasNext()) {
      rowCount++;
      Struct* pst = dynamic_cast<Struct*>(itr.next().get());
      LOGINFO("Row %d Column 1 is named %s, value is %s", rowCount,
              pst->getFieldName(0).c_str(), (*pst)[0]->toString()->asChar());
      LOGINFO("Row %d Column 2 is named %s, value is %s", rowCount,
              pst->getFieldName(1).c_str(), (*pst)[1]->toString()->asChar());
    }

    // Close the Geode Cache.
    cachePtr->close();

    LOGINFO("Closed the Geode Cache");

    return 0;

  }
  // An exception should not occur
  catch (const Exception& geodeExcp) {
    LOGERROR("RemoteQuery Geode Exception: %s", geodeExcp.getMessage());

    return 1;
  }
}
