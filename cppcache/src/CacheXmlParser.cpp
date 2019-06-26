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

#include "CacheXmlParser.hpp"

#include <chrono>

#include <geode/PoolFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/internal/chrono/duration.hpp>

#include "AutoDelete.hpp"
#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "util/string.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace apache {
namespace geode {
namespace client {

namespace impl {
void *getFactoryFunc(const std::string &lib, const std::string &funcName);
}  // namespace impl

namespace {
/** The name of the <code>cache</code> element */
auto CACHE = "cache";
auto CLIENT_CACHE = "client-cache";
auto PDX = "pdx";

/** The name of the <code>redundancy-level</code> element */
auto REDUNDANCY_LEVEL = "redundancy-level";

/** The name of the <code>region</code> element */
auto REGION = "region";

/** The name of the <code>root-region</code> element */
auto ROOT_REGION = "root-region";

/** The name of the <code>region-attributes</code> element */
auto REGION_ATTRIBUTES = "region-attributes";

auto LRU_ENTRIES_LIMIT = "lru-entries-limit";

auto DISK_POLICY = "disk-policy";

auto ENDPOINTS = "endpoints";

/** The name of the <code>region-time-to-live</code> element */
auto REGION_TIME_TO_LIVE = "region-time-to-live";

/** The name of the <code>region-idle-time</code> element */
auto REGION_IDLE_TIME = "region-idle-time";

/** The name of the <code>entry-time-to-live</code> element */
auto ENTRY_TIME_TO_LIVE = "entry-time-to-live";

/** The name of the <code>entry-idle-time</code> element */
auto ENTRY_IDLE_TIME = "entry-idle-time";

/** The name of the <code>expiration-attributes</code> element */
auto EXPIRATION_ATTRIBUTES = "expiration-attributes";

/** The name of the <code>cache-loader</code> element */
auto CACHE_LOADER = "cache-loader";

/** The name of the <code>cache-writer</code> element */
auto CACHE_WRITER = "cache-writer";

/** The name of the <code>cache-listener</code> element */
auto CACHE_LISTENER = "cache-listener";

/** The name of the <code>partition-resolver</code> element */
auto PARTITION_RESOLVER = "partition-resolver";

auto LIBRARY_NAME = "library-name";

auto LIBRARY_FUNCTION_NAME = "library-function-name";

auto CACHING_ENABLED = "caching-enabled";

auto INTEREST_LIST_ENABLED = "interest-list-enabled";

auto MAX_DISTRIBUTE_VALUE_LENGTH_WHEN_CREATE =
    "max-distribute-value-length-when-create";
/** The name of the <code>scope</code> attribute */
auto SCOPE = "scope";

/** The name of the <code>client-notification</code> attribute */
auto CLIENT_NOTIFICATION_ENABLED = "client-notification";

/** The name of the <code>initial-capacity</code> attribute */
auto INITIAL_CAPACITY = "initial-capacity";

/** The name of the <code>initial-capacity</code> attribute */
auto CONCURRENCY_LEVEL = "concurrency-level";

/** The name of the <code>load-factor</code> attribute */
auto LOAD_FACTOR = "load-factor";

/** The name of the <code>statistics-enabled</code> attribute */
auto STATISTICS_ENABLED = "statistics-enabled";

/** The name of the <code>timeout</code> attribute */
auto TIMEOUT = "timeout";

/** The name of the <code>action</code> attribute */
auto ACTION = "action";

/** The name of the <code>local</code> value */
auto LOCAL = "local";

/** The name of the <code>distributed-no-ack</code> value */
auto DISTRIBUTED_NO_ACK = "distributed-no-ack";

/** The name of the <code>distributed-ack</code> value */
auto DISTRIBUTED_ACK = "distributed-ack";

/** The name of the <code>global</code> value */
auto GLOBAL = "global";

/** The name of the <code>invalidate</code> value */
auto INVALIDATE = "invalidate";

/** The name of the <code>destroy</code> value */
auto DESTROY = "destroy";

/** The name of the <code>overflow</code> value */
auto OVERFLOWS = "overflows";

/** The name of the <code>overflow</code> value */
auto PERSIST = "persist";

/** The name of the <code>none</code> value */
auto NONE = "none";

/** The name of the <code>local-invalidate</code> value */
auto LOCAL_INVALIDATE = "local-invalidate";

/** The name of the <code>local-destroy</code> value */
auto LOCAL_DESTROY = "local-destroy";

/** The name of the <code>persistence-manager</code> value */
auto PERSISTENCE_MANAGER = "persistence-manager";

/** The name of the <code>properties</code> value */
auto PROPERTIES = "properties";

/** The name of the <code>property</code> value */
auto PROPERTY = "property";

auto CONCURRENCY_CHECKS_ENABLED = "concurrency-checks-enabled";

auto TOMBSTONE_TIMEOUT = "tombstone-timeout";

/** Pool elements and attributes */

auto POOL_NAME = "pool-name";
auto POOL = "pool";
auto NAME = "name";
auto LOCATOR = "locator";
auto SERVER = "server";
auto HOST = "host";
auto PORT = "port";
auto IGNORE_UNREAD_FIELDS = "ignore-unread-fields";
auto READ_SERIALIZED = "read-serialized";
auto FREE_CONNECTION_TIMEOUT = "free-connection-timeout";
auto MULTIUSER_SECURE_MODE = "multiuser-authentication";
auto IDLE_TIMEOUT = "idle-timeout";
auto LOAD_CONDITIONING_INTERVAL = "load-conditioning-interval";
auto MAX_CONNECTIONS = "max-connections";
auto MIN_CONNECTIONS = "min-connections";
auto PING_INTERVAL = "ping-interval";
auto UPDATE_LOCATOR_LIST_INTERVAL = "update-locator-list-interval";
auto READ_TIMEOUT = "read-timeout";
auto RETRY_ATTEMPTS = "retry-attempts";
auto SERVER_GROUP = "server-group";
auto SOCKET_BUFFER_SIZE = "socket-buffer-size";
auto STATISTIC_INTERVAL = "statistic-interval";
auto SUBSCRIPTION_ACK_INTERVAL = "subscription-ack-interval";
auto SUBSCRIPTION_ENABLED = "subscription-enabled";
auto SUBSCRIPTION_MTT = "subscription-message-tracking-timeout";
auto SUBSCRIPTION_REDUNDANCY = "subscription-redundancy";
auto THREAD_LOCAL_CONNECTIONS = "thread-local-connections";
auto CLONING_ENABLED = "cloning-enabled";
auto ID = "id";
auto REFID = "refid";
auto PR_SINGLE_HOP_ENABLED = "pr-single-hop-enabled";

using apache::geode::client::impl::getFactoryFunc;

std::vector<std::pair<std::string, int>> parseEndPoints(
    const std::string &str) {
  std::vector<std::pair<std::string, int>> endPoints;
  std::string::size_type start = 0;
  std::string::size_type pos = str.find_first_of(',');
  while (std::string::npos != pos) {
    const std::string endPoint(str.substr(start, pos - start));
    const std::string::size_type split = endPoint.find_last_of(':');
    if (std::string::npos == split) {
      endPoints.push_back(std::pair<std::string, int>(endPoint, 0));
    } else {
      int port = 0;
      try {
        port = std::stoi(endPoint.substr(split + 1));
      } catch (...) {
        // NOP
      }
      endPoints.push_back(
          std::pair<std::string, int>(endPoint.substr(0, split), port));
    }
    start = pos + 1;
    pos = str.find_first_of(',', start);
  }
  return endPoints;
}
}  // namespace

/**
 * warningDebug:
 * @ctxt:  An XML parser context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 *
 * Display and format a warning messages, gives file, line, position and
 * extra parameters.
 */
extern "C" void warningDebug(void *, const char *msg, ...) {
  char logmsg[2048];
  va_list args;
  va_start(args, msg);
  vsprintf(logmsg, msg, args);
  va_end(args);
  LOGWARN("SAX.warning during XML declarative client initialization: %s",
          logmsg);
}

/////////////End of XML Parser Cackllback functions///////////////

///////////////static variables of the class////////////////////////

LibraryCacheLoaderFn CacheXmlParser::managedCacheLoaderFn = nullptr;
LibraryCacheListenerFn CacheXmlParser::managedCacheListenerFn = nullptr;
LibraryPartitionResolverFn CacheXmlParser::managedPartitionResolverFn = nullptr;
LibraryCacheWriterFn CacheXmlParser::managedCacheWriterFn = nullptr;
LibraryPersistenceManagerFn CacheXmlParser::managedPersistenceManagerFn =
    nullptr;

//////////////////////////////////////////////////////////////////

CacheXmlParser::CacheXmlParser(Cache *)
    : m_cacheCreation(nullptr),
      m_nestedRegions(0),
      m_config(nullptr),
      m_parserMessage(""),
      m_flagCacheXmlException(false),
      m_flagIllegalStateException(false),
      m_flagAnyOtherException(false),
      m_flagExpirationAttribute(false),
      namedRegions(CacheImpl::getRegionShortcut()),
      m_poolFactory(nullptr) {
//      m_cache(cache) {
  start_element_map_.emplace(
      std::make_tuple(std::string(CACHE), &CacheXmlParser::startCache));
  start_element_map_.emplace(
      std::make_tuple(std::string(CLIENT_CACHE), &CacheXmlParser::startCache));
  start_element_map_.emplace(
      std::make_tuple(std::string(PDX), &CacheXmlParser::startPdx));
  start_element_map_.emplace(
      std::make_tuple(std::string(REGION), &CacheXmlParser::startRegion));
  start_element_map_.emplace(
      std::make_tuple(std::string(ROOT_REGION), &CacheXmlParser::startRegion));
  start_element_map_.emplace(std::make_tuple(
      std::string(REGION_ATTRIBUTES), &CacheXmlParser::startRegionAttributes));
  start_element_map_.emplace(
      std::make_tuple(std::string(EXPIRATION_ATTRIBUTES),
                      &CacheXmlParser::startExpirationAttributes));
  start_element_map_.emplace(std::make_tuple(
      std::string(CACHE_LOADER), &CacheXmlParser::startCacheLoader));
  start_element_map_.emplace(std::make_tuple(
      std::string(CACHE_WRITER), &CacheXmlParser::startCacheWriter));
  start_element_map_.emplace(std::make_tuple(
      std::string(CACHE_LISTENER), &CacheXmlParser::startCacheListener));
  start_element_map_.emplace(
      std::make_tuple(std::string(PARTITION_RESOLVER),
                      &CacheXmlParser::startPartitionResolver));
  start_element_map_.emplace(
      std::make_tuple(std::string(PERSISTENCE_MANAGER),
                      &CacheXmlParser::startPersistenceManager));
  start_element_map_.emplace(std::make_tuple(
      std::string(PROPERTY), &CacheXmlParser::startPersistenceProperties));
  start_element_map_.emplace(
      std::make_tuple(std::string(POOL), &CacheXmlParser::startPool));
  start_element_map_.emplace(
      std::make_tuple(std::string(LOCATOR), &CacheXmlParser::startLocator));
  start_element_map_.emplace(
      std::make_tuple(std::string(SERVER), &CacheXmlParser::startServer));

  end_element_map_.emplace(
      std::make_tuple(std::string(CACHE), &CacheXmlParser::endCache));
  end_element_map_.emplace(
      std::make_tuple(std::string(CLIENT_CACHE), &CacheXmlParser::endCache));
  end_element_map_.emplace(
      std::make_tuple(std::string(PDX), &CacheXmlParser::endPdx));
  end_element_map_.emplace(
      std::make_tuple(std::string(REGION), &CacheXmlParser::endRegion));
  end_element_map_.emplace(
      std::make_tuple(std::string(ROOT_REGION), &CacheXmlParser::endRegion));
  end_element_map_.emplace(std::make_tuple(
      std::string(REGION_ATTRIBUTES), &CacheXmlParser::endRegionAttributes));
  end_element_map_.emplace(std::make_tuple(
      std::string(REGION_TIME_TO_LIVE), &CacheXmlParser::endRegionTimeToLive));
  end_element_map_.emplace(std::make_tuple(std::string(REGION_IDLE_TIME),
                                           &CacheXmlParser::endRegionIdleTime));
  end_element_map_.emplace(std::make_tuple(
      std::string(ENTRY_TIME_TO_LIVE), &CacheXmlParser::endEntryTimeToLive));
  end_element_map_.emplace(std::make_tuple(std::string(ENTRY_IDLE_TIME),
                                           &CacheXmlParser::endEntryIdleTime));
  end_element_map_.emplace(
      std::make_tuple(std::string(PERSISTENCE_MANAGER),
                      &CacheXmlParser::endPersistenceManager));
  end_element_map_.emplace(
      std::make_tuple(std::string(POOL), &CacheXmlParser::endPool));
}

void CacheXmlParser::startElement(const XMLCh *const,
                                  const XMLCh *const localname,
                                  const XMLCh *const,
                                  const xercesc::Attributes &attrs) {
  auto message = xercesc::XMLString::transcode(localname);
  auto name = std::string(message);
  auto iter = start_element_map_.find(name);
  if (iter != std::end(start_element_map_)) {
    iter->second(*this, attrs);
  }
  xercesc::XMLString::release(&message);
}

void CacheXmlParser::endElement(const XMLCh *const,
                                const XMLCh *const localname,
                                const XMLCh *const) {
  auto message = xercesc::XMLString::transcode(localname);
  auto name = std::string(message);
  auto iter = end_element_map_.find(name);
  if (iter != std::end(end_element_map_)) {
    iter->second(*this);
  }
  xercesc::XMLString::release(&message);
}

void CacheXmlParser::fatalError(const xercesc::SAXParseException &exception) {
  using namespace xercesc;
  char *message = XMLString::transcode(exception.getMessage());
  LOGDEBUG("Fatal Error: \"%s\" at line: %ulld", message,
           exception.getLineNumber());
  XMLString::release(&message);
}

void CacheXmlParser::parseFile(const char *filename) {
  int res = 0;

// TODO: replace with Xerces equivalent
//  res = xmlSAXUserParseFile(&this->m_saxHandler, this, filename);

  if (res == -1) {
    throw CacheXmlException("Xml file " + std::string(filename) + " not found");
  }
  handleParserErrors(res);
}

void CacheXmlParser::parseMemory(const char *, int) {
  int res = 0;
// TODO: replace with Xerces equivalent
//  res = xmlSAXUserParseMemory(&this->m_saxHandler, this, buffer, size);

  if (res == -1) {
    throw CacheXmlException("Unable to read buffer.");
  }
  handleParserErrors(res);
}

void CacheXmlParser::handleParserErrors(int res) {
  if (res != 0)  // xml file is not well-formed
  {
    Log::error("Error code returned by xml parser is : " + std::to_string(res));
    throw CacheXmlException("Xml file is not well formed. Error _stack: \n" +
                            std::string(this->m_parserMessage));
  }
  std::string temp = this->getError();
  if (temp.empty()) {
    Log::info("Xml file parsed successfully");
  } else {
    // well formed, but not according to our specs(dtd errors thrown manually)
    if (this->m_flagCacheXmlException) {
      throw CacheXmlException(temp);
    } else if (this->m_flagIllegalStateException) {
      throw IllegalStateException(temp);
    } else if (this->m_flagAnyOtherException) {
      throw UnknownException(temp);
    }
    this->setError("");
  }
}

//////////////////////  Static Methods  //////////////////////

/**
 * Parses XML data and from it creates an instance of
 * <code>CacheXmlParser</code> that can be used to create
 * the {@link Cache}, etc.
 *
 * @param  cacheXml
 *         The xml file
 *
 * @throws CacheXmlException
 *         Something went wrong while parsing the XML
 * @throws OutOfMemoryException
 * @throws CacheXmlException
 *         If xml file is not well-formed or
 *         Something went wrong while parsing the XML
 * @throws IllegalStateException
 *         If xml file is well-flrmed but not valid
 * @throws UnknownException otherwise
 */
CacheXmlParser *CacheXmlParser::parse(const char *cacheXml, Cache *cache) {
  CacheXmlParser *handler;
  _GEODE_NEW(handler, CacheXmlParser(cache));
  // use RAII to delete the handler object in case of exceptions
  DeleteObject<CacheXmlParser> delHandler(handler);

  {
    handler->parseFile(cacheXml);
    delHandler.noDelete();
    return handler;
  }
}

void CacheXmlParser::setAttributes(Cache *) {}

/**
 * Creates cache artifacts ({@link Cache}s, etc.) based upon the XML
 * parsed by this parser.
 *
 * @param  cache
 *         The cachewhcih is to be populated
 * @throws OutOfMemoryException if the memory allocation failed
 * @throws NotConnectedException if the cache is not connected
 * @throws InvalidArgumentException if the attributePtr is nullptr.
 * or if RegionAttributes is null or if regionName is null,
 * the empty   string, or contains a '/'
 * @throws RegionExistsException
 * @throws CacheClosedException if the cache is closed
 *         at the time of region creation
 * @throws UnknownException otherwise
 *
 */
void CacheXmlParser::create(Cache *cache) {
  // use DeleteObject class to delete m_cacheCreation in case of exceptions
  DeleteObject<CacheXmlCreation> delCacheCreation(m_cacheCreation);

  if (cache == nullptr) {
    throw IllegalArgumentException(
        "XML:No cache specified for performing configuration");
  }
  if (!m_cacheCreation) {
    throw CacheXmlException("XML: Element <cache> was not provided in the xml");
  }
  m_cacheCreation->create(cache);
  delCacheCreation.noDelete();
  Log::info("Declarative configuration of cache completed successfully");
}

void CacheXmlParser::startCache(const xercesc::Attributes &attrs) {
  using unique_xml_char = std::unique_ptr<XMLCh, std::function<void(XMLCh *)>>;
  using unique_xerces_char = std::unique_ptr<char, std::function<void(char *)>>;
  auto xml_deleter = [](XMLCh *ch) { xercesc::XMLString::release(&ch); };
  auto xerces_deleter = [](char *ch) { xercesc::XMLString::release(&ch); };

  unique_xml_char endpoints(xercesc::XMLString::transcode(ENDPOINTS),
                            xml_deleter);
  unique_xml_char redundancy_level(
      xercesc::XMLString::transcode(REDUNDANCY_LEVEL), xml_deleter);

  if (auto value = attrs.getValue(endpoints.get())) {
    if (m_poolFactory) {
      unique_xerces_char v(xercesc::XMLString::transcode(value),
                           xerces_deleter);
      for (auto &&endPoint : parseEndPoints(v.get())) {
        m_poolFactory->addServer(endPoint.first, endPoint.second);
      }
    }
  } else if (auto value = attrs.getValue(redundancy_level.get())) {
    unique_xerces_char v(xercesc::XMLString::transcode(value), xerces_deleter);
    m_poolFactory->setSubscriptionRedundancy(std::stoi(v.get()));
  } else {
    throw CacheXmlException("XML: No value provided for attribute: " +
                            std::string(CACHE) + " or " + CLIENT_CACHE);
  }

  _GEODE_NEW(m_cacheCreation, CacheXmlCreation());
}

void CacheXmlParser::startPdx(const xercesc::Attributes &) {
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//    if (value.empty()) {
//      throw CacheXmlException("XML: No value provided for attribute: " + name);
//    }
//
//    if (IGNORE_UNREAD_FIELDS == name) {
//      bool flag = false;
//      std::transform(value.begin(), value.end(), value.begin(), ::tolower);
//      if ("false" == value) {
//        flag = false;
//      } else if ("true" == value) {
//        flag = true;
//      } else {
//        throw CacheXmlException(
//            "XML: " + value +
//            " is not a valid value for the attribute <ignore-unread-fields>");
//      }
//      m_cacheCreation->setPdxIgnoreUnreadField(flag);
//    } else if (READ_SERIALIZED == name) {
//      bool flag = false;
//      std::transform(value.begin(), value.end(), value.begin(), ::tolower);
//      if ("false" == value) {
//        flag = false;
//      } else if ("true" == value) {
//        flag = true;
//      } else {
//        throw CacheXmlException(
//            "XML: " + value +
//            " is not a valid value for the attribute <ignore-unread-fields>");
//      }
//      m_cacheCreation->setPdxReadSerialized(flag);
//    } else {
//      throw CacheXmlException("XML:Unrecognized pdx attribute " + name);
//    }
//  }
}

void CacheXmlParser::endPdx() {}

void CacheXmlParser::startLocator(const xercesc::Attributes &) {
//  if (!attrs) {
//    throw CacheXmlException(
//        "XML:No attributes provided for <locator>. A locator requires a host "
//        "and port");
//  }
//  m_poolFactory = std::static_pointer_cast<PoolFactory>(_stack.top());
//
//  std::string host;
//  std::string port;
//  int attrsCount = 0;
//
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//    if (value.empty()) {
//      throw CacheXmlException("XML: No value provided for attribute: " + name);
//    }
//
//    if (HOST == name) {
//      host = std::move(value);
//    } else if (PORT == name) {
//      port = std::move(value);
//    } else {
//      throw CacheXmlException("XML:Unrecognized locator attribute");
//    }
//
//    ++attrsCount;
//  }
//
//  if (attrsCount < 2) {
//    throw CacheXmlException(
//        "XML:Not enough attributes provided for a <locator> - host and port "
//        "required");
//  }
//
//  m_poolFactory->addLocator(host, std::stoi(port));
}

void CacheXmlParser::startServer(const xercesc::Attributes &) {
//  if (!attrs) {
//    throw CacheXmlException(
//        "XML:No attributes provided for <server>. A server requires a host and "
//        "port");
//  }
//  auto factory = std::static_pointer_cast<PoolFactory>(_stack.top());
//
//  std::string host;
//  std::string port;
//  int attrsCount = 0;
//
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//    if (value.empty()) {
//      throw CacheXmlException("XML: No value provided for attribute: " + name);
//    }
//
//    if (HOST == name) {
//      host = std::move(value);
//    } else if (PORT == name) {
//      port = std::move(value);
//    } else {
//      throw CacheXmlException("XML:Unrecognized server attribute");
//    }
//
//    ++attrsCount;
//  }
//
//  if (attrsCount < 2) {
//    throw CacheXmlException(
//        "XML:Not enough attributes provided for a <server> - host and port "
//        "required");
//  }
//
//  factory->addServer(host, std::stoi(port));
}

void CacheXmlParser::startPool(const xercesc::Attributes &) {
//  if (!attrs) {
//    throw CacheXmlException(
//        "XML:No attributes provided for <pool>. A pool cannot be created "
//        "without a name");
//  }
//  auto factory =
//      std::make_shared<PoolFactory>(m_cache->getPoolManager().createFactory());
//
//  std::string poolName;
//
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//    if (value.empty()) {
//      throw CacheXmlException("XML: No value provided for attribute: " + name);
//    }
//
//    if (NAME == name) {
//      poolName = std::move(value);
//    } else {
//      setPoolInfo(factory.get(), name.c_str(), value.c_str());
//    }
//  }
//
//  if (poolName.empty()) {
//    throw CacheXmlException(
//        "XML:No attributes provided for a <pool> - at least the name is "
//        "required");
//  }
//
//  auto poolxml = std::make_shared<PoolXmlCreation>(poolName, factory);
//
//  _stack.push(poolxml);
//  _stack.push(factory);
}

void CacheXmlParser::endPool() {
  _stack.pop();  // remove factory
  auto poolxml = std::static_pointer_cast<PoolXmlCreation>(_stack.top());
  _stack.pop();  // remove pool
  m_cacheCreation->addPool(poolxml);
}

void CacheXmlParser::setPoolInfo(PoolFactory *factory, const char *name,
                                 const char *value) {
  using apache::geode::client::equal_ignore_case;
  using apache::geode::internal::chrono::duration::from_string;

  if (strcmp(name, FREE_CONNECTION_TIMEOUT) == 0) {
    factory->setFreeConnectionTimeout(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, MULTIUSER_SECURE_MODE) == 0) {
    if (equal_ignore_case(value, "true")) {
      factory->setMultiuserAuthentication(true);
    } else {
      factory->setMultiuserAuthentication(false);
    }
  } else if (strcmp(name, IDLE_TIMEOUT) == 0) {
    factory->setIdleTimeout(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, LOAD_CONDITIONING_INTERVAL) == 0) {
    factory->setLoadConditioningInterval(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, MAX_CONNECTIONS) == 0) {
    factory->setMaxConnections(atoi(value));
  } else if (strcmp(name, MIN_CONNECTIONS) == 0) {
    factory->setMinConnections(atoi(value));
  } else if (strcmp(name, PING_INTERVAL) == 0) {
    factory->setPingInterval(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, UPDATE_LOCATOR_LIST_INTERVAL) == 0) {
    factory->setUpdateLocatorListInterval(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, READ_TIMEOUT) == 0) {
    factory->setReadTimeout(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, RETRY_ATTEMPTS) == 0) {
    factory->setRetryAttempts(atoi(value));
  } else if (strcmp(name, SERVER_GROUP) == 0) {
    factory->setServerGroup(value);
  } else if (strcmp(name, SOCKET_BUFFER_SIZE) == 0) {
    factory->setSocketBufferSize(atoi(value));
  } else if (strcmp(name, STATISTIC_INTERVAL) == 0) {
    factory->setStatisticInterval(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, SUBSCRIPTION_ACK_INTERVAL) == 0) {
    factory->setSubscriptionAckInterval(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, SUBSCRIPTION_ENABLED) == 0) {
    if (equal_ignore_case(value, "true")) {
      factory->setSubscriptionEnabled(true);
    } else {
      factory->setSubscriptionEnabled(false);
    }
  } else if (strcmp(name, SUBSCRIPTION_MTT) == 0) {
    factory->setSubscriptionMessageTrackingTimeout(
        from_string<std::chrono::milliseconds>(std::string(value)));
  } else if (strcmp(name, SUBSCRIPTION_REDUNDANCY) == 0) {
    factory->setSubscriptionRedundancy(atoi(value));
  } else if (strcmp(name, THREAD_LOCAL_CONNECTIONS) == 0) {
    if (equal_ignore_case(value, "true")) {
      factory->setThreadLocalConnections(true);
    } else {
      factory->setThreadLocalConnections(false);
    }
  } else if (strcmp(name, PR_SINGLE_HOP_ENABLED) == 0) {
    if (equal_ignore_case(value, "true")) {
      factory->setPRSingleHopEnabled(true);
    } else {
      factory->setPRSingleHopEnabled(false);
    }
  } else {
    throw CacheXmlException("XML:Unrecognized pool attribute " +
                            std::string(name));
  }
}

/**
 * When a <code>region</code> element is first encountered, we
 * create a {@link RegionCreation} and push it on the _stack.
 * An {@link RegionAttributesFactory }is also created and puhed on _stack.
 */
void CacheXmlParser::startRegion(const xercesc::Attributes &) {
//  incNesting();
//  auto isRoot = isRootLevel();
//  int attrsCount = 0;
//  if (!attrs) {
//    throw CacheXmlException(
//        "XML:No attributes provided for <region>. A region cannot be created "
//        "without a name");
//  }
//  while (attrs[attrsCount]) ++attrsCount;
//  if (attrsCount < 2 || attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for a <region>");
//  }
//
//  std::string regionName;
//  std::string refid;
//
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//    if ("name" == name) {
//      regionName = value;
//    } else if ("refid" == name) {
//      refid = value;
//    } else {
//      throw CacheXmlException("XML:<region> does not contain the attribute :" +
//                              name);
//    }
//  }
//
//  if (regionName.empty()) {
//    throw CacheXmlException(
//        "XML:The attribute name of <region> should be specified and cannot be "
//        "empty");
//  }
//
//  auto region = std::make_shared<RegionXmlCreation>(regionName, isRoot);
//  if (!region) {
//    throw UnknownException("CacheXmlParser::startRegion:Out of memeory");
//  }
//
//  _stack.push(region);
//
//  RegionAttributesFactory regionAttributesFactory;
//  if (!refid.empty()) {
//    if (namedRegions.find(refid) != namedRegions.end()) {
//      regionAttributesFactory = RegionAttributesFactory(namedRegions[refid]);
//    } else {
//      throw CacheXmlException("XML:referenced named attribute '" + refid +
//                              "' does not exist.");
//    }
//  }
//
//  region->setAttributes(regionAttributesFactory.create());
}

void CacheXmlParser::startRegionAttributes(const xercesc::Attributes &) {
//  bool isDistributed = false;
//  bool isTCR = false;
//  std::shared_ptr<RegionAttributesFactory> regionAttributesFactory = nullptr;
//  if (attrs) {
//    int attrsCount = 0;
//    while (attrs[attrsCount]) ++attrsCount;
//    if (attrsCount > 24)  // Remember to change this when the number changes
//    {
//      throw CacheXmlException(
//          "XML:Number of attributes provided for <region-attributes> are more");
//    }
//
//    std::string refid;
//
//    for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//      auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//      auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//      if (value.empty()) {
//        throw CacheXmlException(
//            "XML:In the <region-attributes> element attribute " + name +
//            " can be set to empty string. It should either have a "
//            "value or the attribute should be removed. In the latter case "
//            "the default value will be set");
//      }
//
//      if (ID == name) {
//        auto region = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
//        region->setAttrId(value);
//      } else if (REFID == name) {
//        refid = std::move(value);
//      }
//    }
//
//    if (refid.empty()) {
//      auto region = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
//      regionAttributesFactory =
//          std::make_shared<RegionAttributesFactory>(region->getAttributes());
//    } else {
//      if (namedRegions.find(refid) != namedRegions.end()) {
//        regionAttributesFactory =
//            std::make_shared<RegionAttributesFactory>(namedRegions[refid]);
//      } else {
//        throw CacheXmlException("XML:referenced named attribute '" + refid +
//                                "' does not exist.");
//      }
//    }
//
//    if (!regionAttributesFactory) {
//      throw UnknownException(
//          "CacheXmlParser::startRegionAttributes:Out of memeory");
//    }
//
//    _stack.push(regionAttributesFactory);
//
//    for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//      auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//      auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//      if (ID == name || REFID == name) {
//        continue;
//      } else if (CLIENT_NOTIFICATION_ENABLED == name) {
//        bool flag = false;
//        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
//        if ("false" == value) {
//          flag = false;
//        } else if ("true" == value) {
//          flag = true;
//        } else {
//          throw CacheXmlException(
//              "XML: " + value +
//              " is not a valid name for the attribute <client-notification>");
//        }
//        if (m_poolFactory) {
//          m_poolFactory->setSubscriptionEnabled(flag);
//        }
//      } else if (INITIAL_CAPACITY == name) {
//        regionAttributesFactory->setInitialCapacity(std::stoi(value));
//      } else if (CONCURRENCY_LEVEL == name) {
//        regionAttributesFactory->setConcurrencyLevel(std::stoi(value));
//      } else if (LOAD_FACTOR == name) {
//        regionAttributesFactory->setLoadFactor(std::stof(value));
//      } else if (CACHING_ENABLED == name) {
//        bool flag = false;
//        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
//        if ("false" == value) {
//          flag = false;
//        } else if ("true" == value) {
//          flag = true;
//        } else {
//          throw CacheXmlException(
//              "XML: " + value +
//              " is not a valid name for the attribute <caching-enabled>");
//        }
//        regionAttributesFactory->setCachingEnabled(flag);
//      } else if (LRU_ENTRIES_LIMIT == name) {
//        regionAttributesFactory->setLruEntriesLimit(std::stoi(value));
//      } else if (DISK_POLICY == name) {
//        auto diskPolicy = apache::geode::client::DiskPolicyType::NONE;
//        if (OVERFLOWS == value) {
//          diskPolicy = apache::geode::client::DiskPolicyType::OVERFLOWS;
//        } else if (PERSIST == value) {
//          throw IllegalStateException("Persistence feature is not supported");
//        } else if (NONE == value) {
//          diskPolicy = apache::geode::client::DiskPolicyType::NONE;
//        } else {
//          throw CacheXmlException(
//              "XML: " + value +
//              " is not a valid name for the attribute <disk-policy>");
//        }
//        regionAttributesFactory->setDiskPolicy(diskPolicy);
//      } else if (ENDPOINTS == name) {
//        if (m_poolFactory) {
//          for (auto &&endPoint : parseEndPoints(name)) {
//            m_poolFactory->addServer(endPoint.first, endPoint.second);
//          }
//        }
//        isTCR = true;
//      } else if (POOL_NAME == name) {
//        regionAttributesFactory->setPoolName(value);
//        isTCR = true;
//      } else if (CLONING_ENABLED == name) {
//        bool flag = false;
//        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
//        if ("false" == value) {
//          flag = false;
//        } else if ("true" == value) {
//          flag = true;
//        } else {
//          throw CacheXmlException(
//              "XML: " + value +
//              " is not a valid value for the attribute <cloning-enabled>");
//        }
//        regionAttributesFactory->setCloningEnabled(flag);
//        isTCR = true;
//      } else if (CONCURRENCY_CHECKS_ENABLED == name) {
//        bool flag = false;
//        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
//        if ("false" == value) {
//          flag = false;
//        } else if ("true" == value) {
//          flag = true;
//        } else {
//          throw CacheXmlException("XML: " + value +
//                                  " is not a valid value for the attribute "
//                                  "<concurrency-checks-enabled>");
//        }
//        regionAttributesFactory->setConcurrencyChecksEnabled(flag);
//      }
//    }  // for loop
//  }    // attrs is nullptr
//  else {
//    auto region = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
//    regionAttributesFactory =
//        std::make_shared<RegionAttributesFactory>(region->getAttributes());
//    _stack.push(regionAttributesFactory);
//  }
//
//  if (isDistributed && isTCR) {
//    /* we don't allow DR+TCR at current stage according to sudhir */
//    throw CacheXmlException(
//        "XML:endpoints cannot be defined for distributed region.\n");
//  }
}

void CacheXmlParser::endRegionAttributes() {
  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  _stack.pop();
  if (!regionAttributesFactory) {
    throw UnknownException(
        "CacheXmlParser::endRegion:RegionAttributesFactory is null");
  }

  auto regionAttributes = regionAttributesFactory->create();

  auto regionPtr = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
  if (!regionPtr) {
    throw UnknownException("CacheXmlParser::endRegion:Region is null");
  }

  std::string id = regionPtr->getAttrId();
  if (id != "") {
    namedRegions[id] = regionAttributes;
  }

  regionPtr->setAttributes(regionAttributes);
}

/**
 * When a <code>expiration-attributes</code> element is first
 * encountered, we create an {@link ExpirationAttibutes} object from
 * the element's attributes and push it on the _stack.
 */
void CacheXmlParser::startExpirationAttributes(
    const xercesc::Attributes &) {
//  using apache::geode::internal::chrono::duration::from_string;
//
//  if (!attrs) {
//    throw CacheXmlException(
//        "XML:No attributes provided for <expiration-attributes> ");
//  }
//  m_flagExpirationAttribute = true;
//  size_t attrsCount = 0;
//  while (attrs[attrsCount]) ++attrsCount;
//  if (attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for "
//        "<expirartion-attributes>");
//  }
//  std::string timeOut;
//  ExpirationAction expire = ExpirationAction::INVALID_ACTION;
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//    if (TIMEOUT == name) {
//      if (value.empty()) {
//        throw CacheXmlException(
//            "XML:Value for attribute <timeout> needs to be specified");
//      }
//      timeOut = value;
//    } else if (ACTION == name) {
//      if (value.empty()) {
//        throw CacheXmlException(
//            "XML:The attribute <action> of <expiration-attributes> cannot be "
//            "set to empty string. It should either have a value or the "
//            "attribute should be removed. In the latter case the default value "
//            "will be set");
//      } else if (INVALIDATE == value) {
//        expire = ExpirationAction::INVALIDATE;
//      } else if (DESTROY == value) {
//        expire = ExpirationAction::DESTROY;
//      } else if (LOCAL_INVALIDATE == value) {
//        expire = ExpirationAction::LOCAL_INVALIDATE;
//      } else if (LOCAL_DESTROY == value) {
//        expire = ExpirationAction::LOCAL_DESTROY;
//      } else {
//        throw CacheXmlException(
//            "XML: " + value +
//            " is not a valid value for the attribute <action>");
//      }
//    } else {
//      throw CacheXmlException(
//          "XML:Incorrect attribute name specified in "
//          "<expiration-attributes>: " +
//          value);
//    }
//  }
//  if (timeOut.empty()) {
//    throw CacheXmlException(
//        "XML:The attribute <timeout> not specified in "
//        "<expiration-attributes>.");
//  }
//  auto timeOutSeconds = from_string<std::chrono::seconds>(timeOut);
//
//  auto expireAttr =
//      std::make_shared<ExpirationAttributes>(timeOutSeconds, expire);
//  if (!expireAttr) {
//    throw UnknownException(
//        "CacheXmlParser::startExpirationAttributes:Out of memory");
//  }
//  expireAttr->setAction(expire);
//
//  _stack.push(expireAttr);
}

void CacheXmlParser::startPersistenceManager(const xercesc::Attributes &) {
//  int attrsCount = 0;
//  if (!attrs) {
//    throw CacheXmlException(
//        "XML:No attributes provided for <persistence-manager>");
//  }
//  while (attrs[attrsCount]) ++attrsCount;
//  if (attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for "
//        "<persistence-manager>");
//  }
//  std::string libraryName;
//  std::string libraryFunctionName;
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//    if (value.empty()) {
//      throw CacheXmlException("XML:Value for attribute <" + name +
//                              "> needs to be specified in the "
//                              "<persistence-manager>");
//    }
//
//    if (LIBRARY_NAME == name) {
//      libraryName = std::move(value);
//    } else if (LIBRARY_FUNCTION_NAME == name) {
//      libraryFunctionName = std::move(value);
//    } else {
//      throw CacheXmlException(
//          "XML:Incorrect attribute name specified in <persistence-manager>: " +
//          value);
//    }
//  }
//  if (libraryFunctionName.empty()) {
//    throw CacheXmlException(
//        "XML:Library function name not specified in the <persistence-manager>");
//  }
//
//  try {
//    if (managedPersistenceManagerFn &&
//        libraryFunctionName.find('.') != std::string::npos) {
//      // this is a managed library
//      (*managedPersistenceManagerFn)(libraryName.c_str(),
//                                     libraryFunctionName.c_str());
//    } else {
//      getFactoryFunc(libraryName, libraryFunctionName);
//    }
//  } catch (IllegalArgumentException &ex) {
//    throw CacheXmlException(ex.what());
//  }
//
//  _stack.emplace(std::make_shared<std::string>(std::move(libraryName)));
//  _stack.emplace(std::make_shared<std::string>(std::move(libraryFunctionName)));
}

void CacheXmlParser::startPersistenceProperties(
    const xercesc::Attributes &) {
//  if (!attrs) {
//    throw CacheXmlException("XML:No attributes provided for <property>");
//  }
//  int attrsCount = 0;
//  while (attrs[attrsCount]) ++attrsCount;
//  if (attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for <property>");
//  } else {
//    if (m_config == nullptr) {
//      m_config = Properties::create();
//    }
//  }
//  std::string propName;
//  std::string propValue;
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//    if (value.empty()) {
//      throw CacheXmlException("XML:Value for attribute <" + name +
//                              "> needs to be specified in the "
//                              "<property>");
//    }
//
//    if ("name" == name) {
//      propName = value;
//    } else if ("value" == name) {
//      propValue = value;
//    } else {
//      throw CacheXmlException(
//          "XML:Incorrect attribute name specified in <property>: " + name);
//    }
//  }
//  if (propName.empty()) {
//    throw CacheXmlException(
//        "XML:attribute <name> needs to be specified in the <property>");
//  }
//  if (propValue.empty()) {
//    throw CacheXmlException(
//        "XML:attribute <value> needs to be  specified in the <property>");
//  }
//  m_config->insert(propName, propValue);
}

void CacheXmlParser::startCacheLoader(const xercesc::Attributes &) {
//  std::string libraryName;
//  std::string libraryFunctionName;
//  int attrsCount = 0;
//  if (!attrs) {
//    throw CacheXmlException("XML:No attributes provided for <cache-loader>");
//  }
//  while (attrs[attrsCount]) ++attrsCount;
//  if (attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for <cache-loader>");
//  }
//
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//    if (value.empty()) {
//      throw CacheXmlException("XML:Value for attribute <" + name +
//                              "> needs to be specified in the "
//                              "<cache-loader>");
//    }
//
//    if (LIBRARY_NAME == name) {
//      libraryName = std::move(value);
//    } else if (LIBRARY_FUNCTION_NAME == name) {
//      libraryFunctionName = std::move(value);
//    } else {
//      throw CacheXmlException(
//          "XML:Incorrect attribute name specified in <cache-loader> : " + name);
//    }
//  }
//  if (libraryFunctionName.empty()) {
//    throw CacheXmlException(
//        "XML:<library-function-name> not specified in <cache-loader>");
//  }
//
//  try {
//    if (managedCacheLoaderFn &&
//        libraryFunctionName.find('.') != std::string::npos) {
//      // this is a managed library
//      (*managedCacheLoaderFn)(libraryName.c_str(), libraryFunctionName.c_str());
//    } else {
//      getFactoryFunc(libraryName, libraryFunctionName);
//    }
//  } catch (IllegalArgumentException &ex) {
//    throw CacheXmlException(ex.what());
//  }
//
//  auto regionAttributesFactory =
//      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
//  regionAttributesFactory->setCacheLoader(libraryName, libraryFunctionName);
}

void CacheXmlParser::startCacheListener(const xercesc::Attributes &) {
//  std::string libraryName;
//  std::string libraryFunctionName;
//  int attrsCount = 0;
//  if (!attrs) {
//    throw CacheXmlException("XML:No attributes provided for <cache-listener>");
//  }
//  while (attrs[attrsCount]) ++attrsCount;
//  if (attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for <cache-listener>");
//  }
//
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//    if (value.empty()) {
//      throw CacheXmlException("XML:Value for attribute <" + name +
//                              "> needs to be specified in the "
//                              "<cache-listener>");
//    }
//
//    if (LIBRARY_NAME == name) {
//      libraryName = std::move(value);
//    } else if (LIBRARY_FUNCTION_NAME == name) {
//      libraryFunctionName = std::move(value);
//    } else {
//      throw CacheXmlException(
//          "XML:Incorrect attribute name specified in <cache-listener> : " +
//          name);
//    }
//  }
//  if (libraryFunctionName.empty()) {
//    throw CacheXmlException(
//        "XML:Library function name not specified in <cache-listener>");
//  }
//
//  try {
//    if (managedCacheListenerFn &&
//        libraryFunctionName.find('.') != std::string::npos) {
//      // this is a managed library
//      (*managedCacheListenerFn)(libraryName.c_str(),
//                                libraryFunctionName.c_str());
//    } else {
//      getFactoryFunc(libraryName, libraryFunctionName);
//    }
//  } catch (IllegalArgumentException &ex) {
//    throw CacheXmlException(ex.what());
//  }
//
//  auto regionAttributesFactory =
//      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
//  regionAttributesFactory->setCacheListener(libraryName, libraryFunctionName);
}

void CacheXmlParser::startPartitionResolver(const xercesc::Attributes &) {
//  if (!attrs) {
//    throw CacheXmlException(
//        "XML:No attributes provided for <partition-resolver> ");
//  }
//
//  int attrsCount = 0;
//  while (attrs[attrsCount]) ++attrsCount;
//  if (attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for "
//        "<partition-resolver>");
//  }
//
//  std::string libraryName;
//  std::string libraryFunctionName;
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//    if (value.empty()) {
//      throw CacheXmlException("XML:Value for attribute <" + name +
//                              "> needs to be specified in the "
//                              "<cache-listener>");
//    }
//
//    if (LIBRARY_NAME == name) {
//      libraryName = std::move(value);
//    } else if (LIBRARY_FUNCTION_NAME == name) {
//      libraryFunctionName = std::move(value);
//    } else {
//      throw CacheXmlException(
//          "XML:Incorrect attribute name specified in <partition-resolver>: " +
//          value);
//    }
//  }
//  if (libraryFunctionName.empty()) {
//    throw CacheXmlException(
//        "XML:Library function name not specified in <partition-resolver> ");
//  }
//
//  try {
//    if (managedPartitionResolverFn != nullptr &&
//        libraryFunctionName.find('.') != std::string::npos) {
//      // this is a managed library
//      (*managedPartitionResolverFn)(libraryName.c_str(),
//                                    libraryFunctionName.c_str());
//    } else {
//      getFactoryFunc(libraryName, libraryFunctionName);
//    }
//  } catch (IllegalArgumentException &ex) {
//    throw CacheXmlException(ex.what());
//  }
//
//  auto regionAttributesFactory =
//      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
//  regionAttributesFactory->setPartitionResolver(libraryName,
//                                                libraryFunctionName);
}

void CacheXmlParser::startCacheWriter(const xercesc::Attributes &) {
//  if (!attrs) {
//    throw CacheXmlException("XML:No attributes provided for <cache-writer>");
//  }
//
//  int attrsCount = 0;
//  while (attrs[attrsCount] != nullptr) ++attrsCount;
//  if (attrsCount > 4) {
//    throw CacheXmlException(
//        "XML:Incorrect number of attributes provided for <cache-writer>");
//  }
//
//  std::string libraryName;
//  std::string libraryFunctionName;
//
//  for (size_t i = 0; attrs[i] && attrs[i + 1]; i += 2) {
//    auto name = std::string(reinterpret_cast<const char *>(attrs[i]));
//    auto value = std::string(reinterpret_cast<const char *>(attrs[i + 1]));
//
//    if (value.empty()) {
//      throw CacheXmlException("XML:Value for attribute <" + name +
//                              "> needs to be specified in the <cache-writer>");
//    }
//
//    if (LIBRARY_NAME == name) {
//      libraryName = std::move(value);
//    } else if (LIBRARY_FUNCTION_NAME == name) {
//      libraryFunctionName = std::move(value);
//    } else {
//      throw CacheXmlException(
//          "XML:Incorrect attribute name specified in <cache-writer>: " + value);
//    }
//  }
//  if (libraryFunctionName.empty()) {
//    throw CacheXmlException(
//        "XML:Library function name not specified in the <cache-writer>");
//  }
//
//  try {
//    if (managedCacheWriterFn &&
//        libraryFunctionName.find('.') != std::string::npos) {
//      // this is a managed library
//      (*managedCacheWriterFn)(libraryName.c_str(), libraryFunctionName.c_str());
//    } else {
//      getFactoryFunc(libraryName, libraryFunctionName);
//    }
//  } catch (IllegalArgumentException &ex) {
//    throw CacheXmlException(ex.what());
//  }
//
//  auto regionAttributesFactory =
//      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
//  regionAttributesFactory->setCacheWriter(libraryName, libraryFunctionName);
}

/**
 * After popping the current <code>RegionXmlCreation</code> off the
 * _stack, if the element on top of the _stack is a
 * <code>RegionXmlCreation</code>, then it is the parent region.
 */
void CacheXmlParser::endRegion() {
  auto isRoot = isRootLevel();
  auto regionPtr = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
  _stack.pop();
  if (isRoot) {
    if (!_stack.empty()) {
      throw CacheXmlException("Xml file has incorrectly nested region tags");
    }
    if (!m_cacheCreation) {
      throw CacheXmlException(
          "XML: Element <cache> was not provided in the xml");
    }

    m_cacheCreation->addRootRegion(regionPtr);
  } else {
    if (_stack.empty()) {
      throw CacheXmlException("Xml file has incorrectly nested region tags");
    }
    auto parent = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
    parent->addSubregion(regionPtr);
  }
  decNesting();
}

/**
 * When a <code>cache</code> element is finished
 */
void CacheXmlParser::endCache() {}

/**
 * When a <code>region-time-to-live</code> element is finished, the
 * {@link ExpirationAttributes} are on top of the _stack followed by
 * the {@link RegionAttributesFactory} to which the expiration
 * attributes are assigned.
 */
void CacheXmlParser::endRegionTimeToLive() {
  if (!m_flagExpirationAttribute) {
    throw CacheXmlException(
        "XML: <region-time-to-live> cannot be without a "
        "<expiration-attributes>");
  }

  auto expireAttr =
      std::static_pointer_cast<ExpirationAttributes>(_stack.top());
  _stack.pop();

  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  regionAttributesFactory->setRegionTimeToLive(expireAttr->getAction(),
                                               expireAttr->getTimeout());
  m_flagExpirationAttribute = false;
}

/**
 * When a <code>region-idle-time</code> element is finished, the
 * {@link ExpirationAttributes} are on top of the _stack followed by
 * the {@link RegionAttributesFactory} to which the expiration
 * attributes are assigned.
 */
void CacheXmlParser::endRegionIdleTime() {
  if (!m_flagExpirationAttribute) {
    throw CacheXmlException(
        "XML: <region-idle-time> cannot be without <expiration-attributes>");
  }
  auto expireAttr =
      std::static_pointer_cast<ExpirationAttributes>(_stack.top());
  _stack.pop();
  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());

  regionAttributesFactory->setRegionIdleTimeout(expireAttr->getAction(),
                                                expireAttr->getTimeout());
  m_flagExpirationAttribute = false;
}

/**
 * When a <code>entry-time-to-live</code> element is finished, the
 * {@link ExpirationAttributes} are on top of the _stack followed by
 * the {@link RegionAttributesFactory} to which the expiration
 * attributes are assigned.
 */
void CacheXmlParser::endEntryTimeToLive() {
  if (!m_flagExpirationAttribute) {
    throw CacheXmlException(
        "XML: <entry-time-to-live> cannot be without <expiration-attributes>");
  }
  auto expireAttr =
      std::static_pointer_cast<ExpirationAttributes>(_stack.top());
  _stack.pop();
  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());

  regionAttributesFactory->setEntryTimeToLive(expireAttr->getAction(),
                                              expireAttr->getTimeout());
  m_flagExpirationAttribute = false;
}

/**
 * When a <code>entry-idle-time</code> element is finished, the
 * {@link ExpirationAttributes} are on top of the _stack followed by
 * the {@link RegionAttributesFactory} to which the expiration
 * attributes are assigned.
 */
void CacheXmlParser::endEntryIdleTime() {
  if (!m_flagExpirationAttribute) {
    throw CacheXmlException(
        "XML: <entry-idle-time> cannot be without <expiration-attributes>");
  }
  auto expireAttr =
      std::static_pointer_cast<ExpirationAttributes>(_stack.top());
  _stack.pop();
  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  // TODO GEODE-3136: consider string parser here.
  regionAttributesFactory->setEntryIdleTimeout(expireAttr->getAction(),
                                               expireAttr->getTimeout());
  m_flagExpirationAttribute = false;
}

/**
 * When persistence-manager attributes is finished, it will set the attribute
 * factory.
 */
void CacheXmlParser::endPersistenceManager() {
  std::shared_ptr<std::string> libraryFunctionName =
      std::static_pointer_cast<std::string>(_stack.top());
  _stack.pop();
  std::shared_ptr<std::string> libraryName =
      std::static_pointer_cast<std::string>(_stack.top());
  _stack.pop();
  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  if (m_config != nullptr) {
    regionAttributesFactory->setPersistenceManager(
        libraryName->c_str(), libraryFunctionName->c_str(), m_config);
    m_config = nullptr;
  } else {
    regionAttributesFactory->setPersistenceManager(
        libraryName->c_str(), libraryFunctionName->c_str());
  }
}

CacheXmlParser::~CacheXmlParser() { _GEODE_SAFE_DELETE(m_cacheCreation); }

void CacheXmlParser::setError(const std::string &err) { m_error = err; }

const std::string &CacheXmlParser::getError() const { return m_error; }

}  // namespace client
}  // namespace geode
}  // namespace apache
