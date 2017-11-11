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
 * The Pool Continuous Query QuickStart Example.
 *
 * This example takes the following steps:
 *
 * 1. Create CacheFactory using the user specified properties or from the
 * geode.properties file by default.
 * 2. Create a Geode Cache.
 * 3. Get the Portfolios Region from the Pool.
 * 4. Populate some query objects on the Region.
 * 5. Get the Query Service from cache.
 * 6. Register a cqQuery listener
 * 7. Execute a cqQuery with initial Results
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

class MyCqListener : public CqListener {
 public:
  void onEvent(const CqEvent& cqe) {
    char* opStr = (char*)"Default";
    std::shared_ptr<Portfolio> portfolio(
        dynamic_cast<Portfolio*>(cqe.getNewValue().get()));
    std::shared_ptr<CacheableString> key(
        dynamic_cast<CacheableString*>(cqe.getKey().get()));
    switch (cqe.getQueryOperation()) {
      case CqOperation::OP_TYPE_CREATE: {
        opStr = (char*)"CREATE";
        break;
      }
      case CqOperation::OP_TYPE_UPDATE: {
        opStr = (char*)"UPDATE";
        break;
      }
      case CqOperation::OP_TYPE_DESTROY: {
        opStr = (char*)"UPDATE";
        break;
      }
      default:
        break;
    }
    LOGINFO("MyCqListener::OnEvent called with %s, key[%s], value=(%ld,%s)",
            opStr, key->asChar(), portfolio->getID(),
            portfolio->getPkid()->asChar());
  }

  void onError(const CqEvent& cqe) { LOGINFO("MyCqListener::OnError called"); }

  void close() { LOGINFO("MyCqListener::close called"); }
};

// The PoolCqQuery QuickStart example.
int main(int argc, char** argv) {
  try {
    // Create CacheFactory using the user specified properties or from the
    // geode.properties file by default.
    auto prp = Properties::create();
    prp->insert("cache-xml-file", "XMLs/clientPoolCqQuery.xml");

    std::shared_ptr<CacheFactory> cacheFactory =
        CacheFactory::createCacheFactory(prp);

    LOGINFO("Created CacheFactory");

    // Create a Geode Cache with the "clientPoolCqQuery.xml" Cache XML file.
    auto cachePtr = cacheFactory->create();

    LOGINFO("Created the Geode Cache");

    // Get the Portfolios Region from the Cache which is declared in the Cache
    // XML file.
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

    // Create CqAttributes and Install Listener
    CqAttributesFactory cqFac;
    std::shared_ptr<CqListener> cqLstner(new MyCqListener());
    cqFac.addCqListener(cqLstner);
    auto cqAttr = cqFac.create();

    // create a new Cq Query
    const char* qryStr = "select * from /Portfolios p where p.ID < 5";
    auto qry = qrySvcPtr->newCq((char*)"MyCq", qryStr, cqAttr);

    // execute Cq Query with initial Results
    auto resultsPtr = qry->executeWithInitialResults();

    // make change to generate cq events
    regionPtr->put("Key3", port1Ptr);
    regionPtr->put("Key2", port2Ptr);
    regionPtr->put("Key1", port3Ptr);

    LOGINFO("ResultSet Query returned %d rows", resultsPtr->size());

    // Iterate through the rows of the query result.
    SelectResultsIterator iter = resultsPtr->getIterator();
    while (iter.hasNext()) {
      std::shared_ptr<Serializable> ser = iter.next();
      if (ser != nullptr) {
        LOGINFO(" query pulled object %s\n", ser->toString()->asChar());
        std::shared_ptr<Struct> stPtr(dynamic_cast<Struct*>(ser.get()));
        if (stPtr != nullptr) {
          LOGINFO(" got struct ptr ");
          std::shared_ptr<Serializable> serKey = (*(stPtr.get()))["key"];
          if (serKey != nullptr) {
            LOGINFO("got struct key %s\n", serKey->toString()->asChar());
          }

          std::shared_ptr<Serializable> serVal = (*(stPtr.get()))["value"];

          if (serVal != nullptr) {
            LOGINFO("  got struct value %s\n", serVal->toString()->asChar());
          }
        }
      } else {
        LOGINFO("   query pulled bad object\n");
      }
    }

    // Stop the Geode Continuous query.
    qry->stop();

    // Close the Geode Continuous query.
    qry->close();

    // Close the Geode Cache.
    cachePtr->close();

    LOGINFO("Closed the Geode Cache");

    return 0;

  }
  // An exception should not occur
  catch (const Exception& geodeExcp) {
    LOGERROR("PoolCqQuery Geode Exception: %s", geodeExcp.getMessage());

    return 1;
  }
}
