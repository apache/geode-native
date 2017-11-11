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
 * The PoolRemoteQuery QuickStart Example.
 * This examples creates pool using locator.
 * This example takes the following steps:
 *
 * 1. Create a Geode Cache.
 * 2. Get the example Region from the Pool.
 * 3. Populate some query objects on the Region.
 * 4. Get the pool, get the Query Service from Cache. Pool is define in
 * clientRemoteQueryWithPool.xml. Pool has locator to get the server. Apart from
 * that pool is bind to server group "ServerGroup1".
 * 5. Execute a query that returns a Result Set.
 * 6. Execute a query that returns a Struct Set.
 * 7. Execute the region shortcut/convenience query methods.
 * 8. Close the Cache.
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

// The PoolRemoteQuery QuickStart example.
int main(int argc, char** argv) {
  try {
    std::shared_ptr<CacheFactory> cacheFactory =
        CacheFactory::createCacheFactory();

    // Create a Geode Cache with the "clientRemoteQueryWithPool.xml" Cache XML
    // file.
    auto cachePtr =
        cacheFactory->set("cache-xml-file", "XMLs/clientPoolRemoteQuery.xml")
            ->create();

    LOGINFO("Created the Geode Cache");

    // Get the example Region from the Cache which is declared in the Cache XML
    // file.
    auto regionPtr = cachePtr->getRegion("Portfolios");

    LOGINFO("Obtained the Region from the Cache");

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
    auto qrySvcPtr = cachePtr->getQueryService("examplePool");

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

    // Close the Geode Cache.
    cachePtr->close();

    LOGINFO("Closed the Geode Cache");

    return 0;

  }
  // An exception should not occur
  catch (const Exception& geodeExcp) {
    LOGERROR("PoolRemoteQuery Geode Exception: %s", geodeExcp.getMessage());

    return 1;
  }
}
