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
#include <cinttypes>

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

/** The name of the <code>property</code> value */
auto PROPERTY = "property";

auto CONCURRENCY_CHECKS_ENABLED = "concurrency-checks-enabled";

auto TOMBSTONE_TIMEOUT = "tombstone-timeout";

/** Pool elements and attributes */

auto POOL_NAME = "pool-name";
auto POOL = "pool";
auto NAME = "name";
auto VALUE = "value";
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

FactoryLoaderFn<CacheLoader> CacheXmlParser::managedCacheLoaderFn_ = nullptr;
FactoryLoaderFn<CacheListener> CacheXmlParser::managedCacheListenerFn_ =
    nullptr;
FactoryLoaderFn<PartitionResolver> CacheXmlParser::managedPartitionResolverFn_ =
    nullptr;
FactoryLoaderFn<CacheWriter> CacheXmlParser::managedCacheWriterFn_ = nullptr;
FactoryLoaderFn<PersistenceManager>
    CacheXmlParser::managedPersistenceManagerFn_ = nullptr;

//////////////////////////////////////////////////////////////////

CacheXmlParser::CacheXmlParser(Cache *cache)
    : cacheCreation_(nullptr),
      nestedRegions_(0),
      config_(nullptr),
      parserMessage_(""),
      flagCacheXmlException_(false),
      flagIllegalStateException_(false),
      flagAnyOtherException_(false),
      flagExpirationAttribute_(false),
      namedRegions_(CacheImpl::getRegionShortcut()),
      poolFactory_(nullptr),
      cache_(cache) {
  start_element_map_.emplace(
      std::make_pair(std::string(CACHE), &CacheXmlParser::startCache));
  start_element_map_.emplace(
      std::make_pair(std::string(CLIENT_CACHE), &CacheXmlParser::startCache));
  start_element_map_.emplace(
      std::make_pair(std::string(PDX), &CacheXmlParser::startPdx));
  start_element_map_.emplace(
      std::make_pair(std::string(REGION), &CacheXmlParser::startRegion));
  start_element_map_.emplace(
      std::make_pair(std::string(ROOT_REGION), &CacheXmlParser::startRegion));
  start_element_map_.emplace(std::make_pair(
      std::string(REGION_ATTRIBUTES), &CacheXmlParser::startRegionAttributes));
  start_element_map_.emplace(
      std::make_pair(std::string(EXPIRATION_ATTRIBUTES),
                     &CacheXmlParser::startExpirationAttributes));
  start_element_map_.emplace(std::make_pair(std::string(CACHE_LOADER),
                                            &CacheXmlParser::startCacheLoader));
  start_element_map_.emplace(std::make_pair(std::string(CACHE_WRITER),
                                            &CacheXmlParser::startCacheWriter));
  start_element_map_.emplace(std::make_pair(
      std::string(CACHE_LISTENER), &CacheXmlParser::startCacheListener));
  start_element_map_.emplace(
      std::make_pair(std::string(PARTITION_RESOLVER),
                     &CacheXmlParser::startPartitionResolver));
  start_element_map_.emplace(
      std::make_pair(std::string(PERSISTENCE_MANAGER),
                     &CacheXmlParser::startPersistenceManager));
  start_element_map_.emplace(std::make_pair(
      std::string(PROPERTY), &CacheXmlParser::startPersistenceProperty));
  start_element_map_.emplace(
      std::make_pair(std::string(POOL), &CacheXmlParser::startPool));
  start_element_map_.emplace(
      std::make_pair(std::string(LOCATOR), &CacheXmlParser::startLocator));
  start_element_map_.emplace(
      std::make_pair(std::string(SERVER), &CacheXmlParser::startServer));

  end_element_map_.emplace(
      std::make_pair(std::string(CACHE), &CacheXmlParser::endCache));
  end_element_map_.emplace(
      std::make_pair(std::string(CLIENT_CACHE), &CacheXmlParser::endCache));
  end_element_map_.emplace(
      std::make_pair(std::string(REGION), &CacheXmlParser::endRegion));
  end_element_map_.emplace(
      std::make_pair(std::string(ROOT_REGION), &CacheXmlParser::endRegion));
  end_element_map_.emplace(std::make_pair(
      std::string(REGION_ATTRIBUTES), &CacheXmlParser::endRegionAttributes));
  end_element_map_.emplace(std::make_pair(
      std::string(REGION_TIME_TO_LIVE), &CacheXmlParser::endRegionTimeToLive));
  end_element_map_.emplace(std::make_pair(std::string(REGION_IDLE_TIME),
                                          &CacheXmlParser::endRegionIdleTime));
  end_element_map_.emplace(std::make_pair(std::string(ENTRY_TIME_TO_LIVE),
                                          &CacheXmlParser::endEntryTimeToLive));
  end_element_map_.emplace(std::make_pair(std::string(ENTRY_IDLE_TIME),
                                          &CacheXmlParser::endEntryIdleTime));
  end_element_map_.emplace(
      std::make_pair(std::string(PERSISTENCE_MANAGER),
                     &CacheXmlParser::endPersistenceManager));
  end_element_map_.emplace(
      std::make_pair(std::string(POOL), &CacheXmlParser::endPool));
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
  char *message = xercesc::XMLString::transcode(exception.getMessage());
  LOGDEBUG("Fatal Error: \"%s\" at line: %" PRIu64, message,
           exception.getLineNumber());
  auto ex = CacheXmlException(message);
  xercesc::XMLString::release(&message);
  throw ex;
}

void CacheXmlParser::parseFile(const char *filename) {
  try {
    xercesc::XMLPlatformUtils::Initialize();
  } catch (const xercesc::XMLException &toCatch) {
    char *message = xercesc::XMLString::transcode(toCatch.getMessage());
    auto exceptionMessage = "Error parsing XML file: " + std::string(message);
    xercesc::XMLString::release(&message);
    throw CacheXmlException(exceptionMessage);
  }

  auto parser = xercesc::XMLReaderFactory::createXMLReader();

  parser->setFeature(xercesc::XMLUni::fgXercesSchema, false);
  parser->setContentHandler(this);
  parser->setErrorHandler(this);

  try {
    parser->parse(filename);
  } catch (const xercesc::XMLException &toCatch) {
    char *message = xercesc::XMLString::transcode(toCatch.getMessage());
    auto exceptionMessage = "Error parsing XML file: " + std::string(message);
    xercesc::XMLString::release(&message);
    throw CacheXmlException(exceptionMessage);
  } catch (const xercesc::SAXParseException &toCatch) {
    char *message = xercesc::XMLString::transcode(toCatch.getMessage());
    auto exceptionMessage = "Error parsing XML file: " + std::string(message);
    xercesc::XMLString::release(&message);
    throw CacheXmlException(exceptionMessage);
  }

  delete parser;
  xercesc::XMLPlatformUtils::Terminate();
}

void CacheXmlParser::parseMemory(const char *buffer, int size) {
  try {
    xercesc::XMLPlatformUtils::Initialize();
  } catch (const xercesc::XMLException &toCatch) {
    char *message = xercesc::XMLString::transcode(toCatch.getMessage());
    auto exceptionMessage = "Error parsing XML file: " + std::string(message);
    xercesc::XMLString::release(&message);
    throw CacheXmlException(exceptionMessage);
  }

  auto parser = xercesc::XMLReaderFactory::createXMLReader();

  parser->setContentHandler(this);
  parser->setErrorHandler(this);

  try {
    xercesc::MemBufInputSource myxml_buf(
        reinterpret_cast<const XMLByte *>(buffer), size,
        "CacheXmlParser memory source");
    parser->parse(myxml_buf);
  } catch (const xercesc::XMLException &toCatch) {
    char *message = xercesc::XMLString::transcode(toCatch.getMessage());
    auto exceptionMessage = "Error parsing XML file: " + std::string(message);
    xercesc::XMLString::release(&message);
    throw CacheXmlException(exceptionMessage);
  } catch (const xercesc::SAXParseException &toCatch) {
    char *message = xercesc::XMLString::transcode(toCatch.getMessage());
    auto exceptionMessage = "Error parsing XML file: " + std::string(message);
    xercesc::XMLString::release(&message);
    throw CacheXmlException(exceptionMessage);
  }

  delete parser;
  xercesc::XMLPlatformUtils::Terminate();
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
  // use DeleteObject class to delete cacheCreation_ in case of exceptions
  DeleteObject<CacheXmlCreation> delCacheCreation(cacheCreation_);

  if (cache == nullptr) {
    throw IllegalArgumentException(
        "XML:No cache specified for performing configuration");
  }
  if (!cacheCreation_) {
    throw CacheXmlException("XML: Element <cache> was not provided in the xml");
  }
  cacheCreation_->create(cache);
  delCacheCreation.noDelete();
  LOGINFO("Declarative configuration of cache completed successfully");
}

std::string CacheXmlParser::getOptionalAttribute(
    const xercesc::Attributes &attrs, const char *attributeName) {
  using unique_xml_char = std::unique_ptr<XMLCh, std::function<void(XMLCh *)>>;
  using unique_xerces_char = std::unique_ptr<char, std::function<void(char *)>>;
  auto xml_deleter = [](XMLCh *ch) { xercesc::XMLString::release(&ch); };
  auto xerces_deleter = [](char *ch) { xercesc::XMLString::release(&ch); };

  unique_xml_char translatedName(xercesc::XMLString::transcode(attributeName),
                                 xml_deleter);
  auto value = attrs.getValue(translatedName.get());
  if (!value) {
    return "";
  }

  unique_xerces_char translatedValue(xercesc::XMLString::transcode(value),
                                     xerces_deleter);
  if (!strlen(translatedValue.get())) {
    throw CacheXmlException("XML: Empty value provided for attribute: " +
                            std::string(CACHE) + " or " + CLIENT_CACHE);
  }

  return {translatedValue.get()};
}

std::string CacheXmlParser::getRequiredAttribute(
    const xercesc::Attributes &attrs, const char *attributeName) {
  using unique_xml_char = std::unique_ptr<XMLCh, std::function<void(XMLCh *)>>;
  auto xml_deleter = [](XMLCh *ch) { xercesc::XMLString::release(&ch); };

  unique_xml_char translatedName(xercesc::XMLString::transcode(attributeName),
                                 xml_deleter);
  auto value = attrs.getValue(translatedName.get());
  if (!value) {
    throw CacheXmlException("XML: No value provided for required attribute: " +
                            std::string(attributeName));
  }

  return getOptionalAttribute(attrs, attributeName);
}

void CacheXmlParser::startCache(const xercesc::Attributes &attrs) {
  auto value = getOptionalAttribute(attrs, ENDPOINTS);
  if (!value.empty()) {
    if (poolFactory_) {
      for (auto &&endPoint : parseEndPoints(value)) {
        poolFactory_->addServer(endPoint.first, endPoint.second);
      }
    }
  }

  value = getOptionalAttribute(attrs, REDUNDANCY_LEVEL);
  if (!value.empty()) {
    if (poolFactory_) {
      poolFactory_->setSubscriptionRedundancy(std::stoi(value));
    }
  }

  _GEODE_NEW(cacheCreation_, CacheXmlCreation());
}

void CacheXmlParser::startPdx(const xercesc::Attributes &attrs) {
  auto ignoreUnreadFields = getOptionalAttribute(attrs, IGNORE_UNREAD_FIELDS);
  if (!ignoreUnreadFields.empty()) {
    if (equal_ignore_case(ignoreUnreadFields, "true")) {
      cacheCreation_->setPdxIgnoreUnreadField(true);
    } else {
      cacheCreation_->setPdxIgnoreUnreadField(false);
    }
  }

  auto pdxReadSerialized = getOptionalAttribute(attrs, READ_SERIALIZED);
  if (!pdxReadSerialized.empty()) {
    if (equal_ignore_case(pdxReadSerialized, "true")) {
      cacheCreation_->setPdxReadSerialized(true);
    } else {
      cacheCreation_->setPdxReadSerialized(false);
    }
  }
}

void CacheXmlParser::startLocator(const xercesc::Attributes &attrs) {
  poolFactory_ = std::static_pointer_cast<PoolFactory>(_stack.top());

  auto host = getRequiredAttribute(attrs, HOST);
  auto port = getRequiredAttribute(attrs, PORT);

  poolFactory_->addLocator(host, std::stoi(port));
}

void CacheXmlParser::startServer(const xercesc::Attributes &attrs) {
  auto factory = std::static_pointer_cast<PoolFactory>(_stack.top());

  auto host = getRequiredAttribute(attrs, HOST);
  auto port = getRequiredAttribute(attrs, PORT);

  factory->addServer(host, std::stoi(port));
}

void CacheXmlParser::startPool(const xercesc::Attributes &attrs) {
  using apache::geode::client::equal_ignore_case;
  using apache::geode::internal::chrono::duration::from_string;

  auto factory =
      std::make_shared<PoolFactory>(cache_->getPoolManager().createFactory());

  auto poolName = getRequiredAttribute(attrs, NAME);

  auto poolxml = std::make_shared<PoolXmlCreation>(poolName, factory);

  auto freeConnectionTimeout =
      getOptionalAttribute(attrs, FREE_CONNECTION_TIMEOUT);
  if (!freeConnectionTimeout.empty()) {
    factory->setFreeConnectionTimeout(
        from_string<std::chrono::milliseconds>(freeConnectionTimeout));
  }

  auto multiUserSecureMode = getOptionalAttribute(attrs, MULTIUSER_SECURE_MODE);
  if (!multiUserSecureMode.empty()) {
    if (equal_ignore_case(multiUserSecureMode, "true")) {
      factory->setMultiuserAuthentication(true);
    } else {
      factory->setMultiuserAuthentication(false);
    }
  }

  auto idleTimeout = getOptionalAttribute(attrs, IDLE_TIMEOUT);
  if (!idleTimeout.empty()) {
    factory->setIdleTimeout(
        from_string<std::chrono::milliseconds>(idleTimeout));
  }

  auto loadConditioningInterval =
      getOptionalAttribute(attrs, LOAD_CONDITIONING_INTERVAL);
  if (!loadConditioningInterval.empty()) {
    factory->setLoadConditioningInterval(
        from_string<std::chrono::milliseconds>(loadConditioningInterval));
  }

  auto maxConnections = getOptionalAttribute(attrs, MAX_CONNECTIONS);
  if (!maxConnections.empty()) {
    factory->setMaxConnections(atoi(maxConnections.c_str()));
  }

  auto minConnections = getOptionalAttribute(attrs, MIN_CONNECTIONS);
  if (!minConnections.empty()) {
    factory->setMinConnections(atoi(minConnections.c_str()));
  }

  auto pingInterval = getOptionalAttribute(attrs, PING_INTERVAL);
  if (!pingInterval.empty()) {
    factory->setPingInterval(
        from_string<std::chrono::milliseconds>(std::string(pingInterval)));
  }

  auto updateLocatorListInterval =
      getOptionalAttribute(attrs, UPDATE_LOCATOR_LIST_INTERVAL);
  if (!updateLocatorListInterval.empty()) {
    factory->setUpdateLocatorListInterval(
        from_string<std::chrono::milliseconds>(updateLocatorListInterval));
  }

  auto readTimeout = getOptionalAttribute(attrs, READ_TIMEOUT);
  if (!readTimeout.empty()) {
    factory->setReadTimeout(
        from_string<std::chrono::milliseconds>(std::string(readTimeout)));
  }

  auto retryAttempts = getOptionalAttribute(attrs, RETRY_ATTEMPTS);
  if (!retryAttempts.empty()) {
    factory->setRetryAttempts(atoi(retryAttempts.c_str()));
  }

  auto serverGroup = getOptionalAttribute(attrs, SERVER_GROUP);
  if (!serverGroup.empty()) {
    factory->setServerGroup(serverGroup);
  }

  auto socketBufferSize = getOptionalAttribute(attrs, SOCKET_BUFFER_SIZE);
  if (!socketBufferSize.empty()) {
    factory->setSocketBufferSize(atoi(socketBufferSize.c_str()));
  }

  auto statisticInterval = getOptionalAttribute(attrs, STATISTIC_INTERVAL);
  if (!statisticInterval.empty()) {
    factory->setStatisticInterval(
        from_string<std::chrono::milliseconds>(statisticInterval));
  }

  auto subscriptionAckInterval =
      getOptionalAttribute(attrs, SUBSCRIPTION_ACK_INTERVAL);
  if (!subscriptionAckInterval.empty()) {
    factory->setSubscriptionAckInterval(
        from_string<std::chrono::milliseconds>(subscriptionAckInterval));
  }

  auto subscriptionEnabled = getOptionalAttribute(attrs, SUBSCRIPTION_ENABLED);
  if (!subscriptionEnabled.empty()) {
    if (equal_ignore_case(subscriptionEnabled, "true")) {
      factory->setSubscriptionEnabled(true);
    } else {
      factory->setSubscriptionEnabled(false);
    }
  }

  auto subscriptionMessageTrackingTimeout =
      getOptionalAttribute(attrs, SUBSCRIPTION_MTT);
  if (!subscriptionMessageTrackingTimeout.empty()) {
    factory->setSubscriptionMessageTrackingTimeout(
        from_string<std::chrono::milliseconds>(
            subscriptionMessageTrackingTimeout));
  }

  auto subscriptionRedundancy =
      getOptionalAttribute(attrs, SUBSCRIPTION_REDUNDANCY);
  if (!subscriptionRedundancy.empty()) {
    factory->setSubscriptionRedundancy(atoi(subscriptionRedundancy.c_str()));
  }

  auto threadLocalConnections =
      getOptionalAttribute(attrs, THREAD_LOCAL_CONNECTIONS);
  if (!threadLocalConnections.empty()) {
    if (equal_ignore_case(threadLocalConnections, "true")) {
      factory->setThreadLocalConnections(true);
    } else {
      factory->setThreadLocalConnections(false);
    }
  }

  auto prSingleHopEnabled = getOptionalAttribute(attrs, PR_SINGLE_HOP_ENABLED);
  if (!prSingleHopEnabled.empty()) {
    if (equal_ignore_case(prSingleHopEnabled, "true")) {
      factory->setPRSingleHopEnabled(true);
    } else {
      factory->setPRSingleHopEnabled(false);
    }
  }

  _stack.push(poolxml);
  _stack.push(factory);
}

void CacheXmlParser::endPool() {
  _stack.pop();  // remove factory
  auto poolxml = std::static_pointer_cast<PoolXmlCreation>(_stack.top());
  _stack.pop();  // remove pool
  cacheCreation_->addPool(poolxml);
}

/**
 * When a <code>region</code> element is first encountered, we
 * create a {@link RegionCreation} and push it on the _stack.
 * An {@link RegionAttributesFactory }is also created and puhed on _stack.
 */
void CacheXmlParser::startRegion(const xercesc::Attributes &attrs) {
  incNesting();
  auto isRoot = isRootLevel();
  auto regionName = getRequiredAttribute(attrs, NAME);

  auto region = std::make_shared<RegionXmlCreation>(regionName, isRoot);
  if (!region) {
    throw UnknownException("CacheXmlParser::startRegion:Out of memory");
  }

  _stack.push(region);

  auto refid = getOptionalAttribute(attrs, REFID);
  if (!refid.empty()) {
    if (namedRegions_.find(refid) != namedRegions_.end()) {
      auto regionAttributesFactory =
          RegionAttributesFactory(namedRegions_[refid]);
      region->setAttributes(regionAttributesFactory.create());
    } else {
      throw CacheXmlException("XML:referenced named attribute '" + refid +
                              "' does not exist.");
    }
  }
}

void CacheXmlParser::startRegionAttributes(const xercesc::Attributes &attrs) {
  bool isDistributed = false;
  bool isTCR = false;
  std::shared_ptr<RegionAttributesFactory> regionAttributesFactory = nullptr;

  if (attrs.getLength() > 24) {
    throw CacheXmlException(
        "XML:Too many attributes provided for <region-attributes>");
  }

  if (attrs.getLength() == 0) {
    auto region = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
    regionAttributesFactory =
        std::make_shared<RegionAttributesFactory>(region->getAttributes());
  } else {
    auto id = getOptionalAttribute(attrs, ID);
    if (!id.empty()) {
      auto region = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
      region->setAttrId(id);
    }

    auto refid = getOptionalAttribute(attrs, REFID);
    if (refid.empty()) {
      auto region = std::static_pointer_cast<RegionXmlCreation>(_stack.top());
      regionAttributesFactory =
          std::make_shared<RegionAttributesFactory>(region->getAttributes());
    } else {
      if (namedRegions_.find(refid) != namedRegions_.end()) {
        regionAttributesFactory =
            std::make_shared<RegionAttributesFactory>(namedRegions_[refid]);
      } else {
        throw CacheXmlException("XML:referenced named attribute '" + refid +
                                "' does not exist.");
      }
    }

    if (!regionAttributesFactory) {
      throw UnknownException(
          "CacheXmlParser::startRegionAttributes:Out of memory");
    }

    auto clientNotificationEnabled =
        getOptionalAttribute(attrs, CLIENT_NOTIFICATION_ENABLED);
    if (!clientNotificationEnabled.empty()) {
      bool flag = false;
      std::transform(clientNotificationEnabled.begin(),
                     clientNotificationEnabled.end(),
                     clientNotificationEnabled.begin(), ::tolower);
      if ("false" == clientNotificationEnabled) {
        flag = false;
      } else if ("true" == clientNotificationEnabled) {
        flag = true;
      } else {
        throw CacheXmlException(
            "XML: " + clientNotificationEnabled +
            " is not a valid name for the attribute <client-notification>");
      }
      if (poolFactory_) {
        poolFactory_->setSubscriptionEnabled(flag);
      }
    }

    auto initialCapacity = getOptionalAttribute(attrs, INITIAL_CAPACITY);
    if (!initialCapacity.empty()) {
      regionAttributesFactory->setInitialCapacity(std::stoi(initialCapacity));
    }

    auto concurrencyLevel = getOptionalAttribute(attrs, CONCURRENCY_LEVEL);
    if (!concurrencyLevel.empty()) {
      regionAttributesFactory->setConcurrencyLevel(std::stoi(concurrencyLevel));
    }

    auto loadFactor = getOptionalAttribute(attrs, LOAD_FACTOR);
    if (!loadFactor.empty()) {
      regionAttributesFactory->setLoadFactor(std::stof(loadFactor));
    }

    auto cachingEnabled = getOptionalAttribute(attrs, CACHING_ENABLED);
    if (!cachingEnabled.empty()) {
      bool flag = false;
      std::transform(cachingEnabled.begin(), cachingEnabled.end(),
                     cachingEnabled.begin(), ::tolower);
      if ("false" == cachingEnabled) {
        flag = false;
      } else if ("true" == cachingEnabled) {
        flag = true;
      } else {
        throw CacheXmlException(
            "XML: " + cachingEnabled +
            " is not a valid name for the attribute <caching-enabled>");
      }
      regionAttributesFactory->setCachingEnabled(flag);
    }

    auto lruEntriesLimit = getOptionalAttribute(attrs, LRU_ENTRIES_LIMIT);
    if (!lruEntriesLimit.empty()) {
      regionAttributesFactory->setLruEntriesLimit(std::stoi(lruEntriesLimit));
    }

    auto diskPolicyString = getOptionalAttribute(attrs, DISK_POLICY);
    if (!diskPolicyString.empty()) {
      auto diskPolicy = apache::geode::client::DiskPolicyType::NONE;
      if (OVERFLOWS == diskPolicyString) {
        diskPolicy = apache::geode::client::DiskPolicyType::OVERFLOWS;
      } else if (PERSIST == diskPolicyString) {
        throw IllegalStateException("Persistence feature is not supported");
      } else if (NONE == diskPolicyString) {
        diskPolicy = apache::geode::client::DiskPolicyType::NONE;
      } else {
        throw CacheXmlException(
            "XML: " + diskPolicyString +
            " is not a valid name for the attribute <disk-policy>");
      }
      regionAttributesFactory->setDiskPolicy(diskPolicy);
    }

    auto endpoints = getOptionalAttribute(attrs, ENDPOINTS);
    if (!endpoints.empty()) {
      if (poolFactory_) {
        for (auto &&endPoint : parseEndPoints(ENDPOINTS)) {
          poolFactory_->addServer(endPoint.first, endPoint.second);
        }
      }
      isTCR = true;
    }

    auto poolName = getOptionalAttribute(attrs, POOL_NAME);
    if (!poolName.empty()) {
      regionAttributesFactory->setPoolName(poolName);
      isTCR = true;
    }

    auto cloningEnabled = getOptionalAttribute(attrs, CLONING_ENABLED);
    if (!cloningEnabled.empty()) {
      bool flag = false;
      std::transform(cloningEnabled.begin(), cloningEnabled.end(),
                     cloningEnabled.begin(), ::tolower);

      if ("false" == cloningEnabled) {
        flag = false;
      } else if ("true" == cloningEnabled) {
        flag = true;
      } else {
        throw CacheXmlException("XML: " + cloningEnabled +
                                " is not a valid value for the attribute <" +
                                std::string(CLONING_ENABLED) + ">");
      }
      regionAttributesFactory->setCloningEnabled(flag);
      isTCR = true;
    }

    auto concurrencyChecksEnabled =
        getOptionalAttribute(attrs, CONCURRENCY_CHECKS_ENABLED);
    if (!concurrencyChecksEnabled.empty()) {
      bool flag = false;
      std::transform(concurrencyChecksEnabled.begin(),
                     concurrencyChecksEnabled.end(),
                     concurrencyChecksEnabled.begin(), ::tolower);
      if ("false" == concurrencyChecksEnabled) {
        flag = false;
      } else if ("true" == concurrencyChecksEnabled) {
        flag = true;
      } else {
        throw CacheXmlException("XML: " + concurrencyChecksEnabled +
                                " is not a valid value for the attribute "
                                "<" +
                                std::string(CONCURRENCY_CHECKS_ENABLED) + ">");
      }
      regionAttributesFactory->setConcurrencyChecksEnabled(flag);
    }
  }

  if (isDistributed && isTCR) {
    // we don't allow DR+TCR at current stage
    throw CacheXmlException(
        "XML:endpoints cannot be defined for distributed region.\n");
  }

  _stack.push(regionAttributesFactory);
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
    namedRegions_[id] = regionAttributes;
  }

  regionPtr->setAttributes(regionAttributes);
}

/**
 * When a <code>expiration-attributes</code> element is first
 * encountered, we create an {@link ExpirationAttibutes} object from
 * the element's attributes and push it on the _stack.
 */
void CacheXmlParser::startExpirationAttributes(
    const xercesc::Attributes &attrs) {
  using apache::geode::internal::chrono::duration::from_string;

  flagExpirationAttribute_ = true;

  if (attrs.getLength() > 2) {
    throw CacheXmlException(
        "XML:Incorrect number of attributes provided for "
        "<expiration-attributes>");
  }

  ExpirationAction expire = ExpirationAction::INVALID_ACTION;
  auto action = getOptionalAttribute(attrs, ACTION);
  if (action.empty()) {
    throw CacheXmlException(
        "XML:The attribute <action> of <expiration-attributes> cannot be"
        "set to empty string. It should either have a action or the "
        "attribute should be removed. In the latter case the default action "
        "will be set");
  } else if (action == INVALIDATE) {
    expire = ExpirationAction::INVALIDATE;
  } else if (action == DESTROY) {
    expire = ExpirationAction::DESTROY;
  } else if (action == LOCAL_INVALIDATE) {
    expire = ExpirationAction::LOCAL_INVALIDATE;
  } else if (action == LOCAL_DESTROY) {
    expire = ExpirationAction::LOCAL_DESTROY;
  } else {
    throw CacheXmlException("XML: " + action +
                            " is not a valid value for the attribute <action>");
  }

  auto timeOut = getOptionalAttribute(attrs, TIMEOUT);
  if (timeOut.empty()) {
    throw CacheXmlException(
        "XML:Value for attribute <timeout> needs to be specified");
  }

  auto timeOutSeconds = from_string<std::chrono::seconds>(timeOut);

  auto expireAttr =
      std::make_shared<ExpirationAttributes>(timeOutSeconds, expire);
  if (!expireAttr) {
    throw UnknownException(
        "CacheXmlParser::startExpirationAttributes:Out of memory");
  }
  expireAttr->setAction(expire);

  _stack.push(expireAttr);
}

std::string CacheXmlParser::getLibraryName(const xercesc::Attributes &attrs) {
  return getRequiredAttribute(attrs, LIBRARY_NAME);
}

std::string CacheXmlParser::getLibraryFunctionName(
    const xercesc::Attributes &attrs) {
  return getRequiredAttribute(attrs, LIBRARY_FUNCTION_NAME);
}

void CacheXmlParser::startPersistenceManager(const xercesc::Attributes &attrs) {
  auto libraryName = getLibraryName(attrs);
  auto libraryFunctionName = getLibraryFunctionName(attrs);

  verifyFactoryFunction(managedPersistenceManagerFn_, libraryName,
                        libraryFunctionName);

  _stack.emplace(std::make_shared<std::string>(std::move(libraryName)));
  _stack.emplace(std::make_shared<std::string>(std::move(libraryFunctionName)));
}

void CacheXmlParser::startPersistenceProperty(
    const xercesc::Attributes &attrs) {
  auto propertyName = getRequiredAttribute(attrs, NAME);
  auto propertyValue = getRequiredAttribute(attrs, VALUE);

  if (config_ == nullptr) {
    config_ = Properties::create();
  }

  config_->insert(propertyName, propertyValue);
}

void CacheXmlParser::startCacheLoader(const xercesc::Attributes &attrs) {
  auto libraryName = getLibraryName(attrs);
  auto libraryFunctionName = getLibraryFunctionName(attrs);

  verifyFactoryFunction(managedCacheLoaderFn_, libraryName,
                        libraryFunctionName);

  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  regionAttributesFactory->setCacheLoader(libraryName, libraryFunctionName);
}

void CacheXmlParser::startCacheListener(const xercesc::Attributes &attrs) {
  auto libraryName = getLibraryName(attrs);
  auto libraryFunctionName = getLibraryFunctionName(attrs);

  verifyFactoryFunction(managedCacheListenerFn_, libraryName,
                        libraryFunctionName);

  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  regionAttributesFactory->setCacheListener(libraryName, libraryFunctionName);
}

void CacheXmlParser::startPartitionResolver(const xercesc::Attributes &attrs) {
  auto libraryName = getLibraryName(attrs);
  auto libraryFunctionName = getLibraryFunctionName(attrs);

  verifyFactoryFunction(managedPartitionResolverFn_, libraryName,
                        libraryFunctionName);

  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  regionAttributesFactory->setPartitionResolver(libraryName,
                                                libraryFunctionName);
}

void CacheXmlParser::startCacheWriter(const xercesc::Attributes &attrs) {
  auto libraryName = getLibraryName(attrs);
  auto libraryFunctionName = getLibraryFunctionName(attrs);

  verifyFactoryFunction(managedCacheWriterFn_, libraryName,
                        libraryFunctionName);

  auto regionAttributesFactory =
      std::static_pointer_cast<RegionAttributesFactory>(_stack.top());
  regionAttributesFactory->setCacheWriter(libraryName, libraryFunctionName);
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
    if (!cacheCreation_) {
      throw CacheXmlException(
          "XML: Element <cache> was not provided in the xml");
    }

    cacheCreation_->addRootRegion(regionPtr);
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
  if (!flagExpirationAttribute_) {
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
  flagExpirationAttribute_ = false;
}

/**
 * When a <code>region-idle-time</code> element is finished, the
 * {@link ExpirationAttributes} are on top of the _stack followed by
 * the {@link RegionAttributesFactory} to which the expiration
 * attributes are assigned.
 */
void CacheXmlParser::endRegionIdleTime() {
  if (!flagExpirationAttribute_) {
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
  flagExpirationAttribute_ = false;
}

/**
 * When a <code>entry-time-to-live</code> element is finished, the
 * {@link ExpirationAttributes} are on top of the _stack followed by
 * the {@link RegionAttributesFactory} to which the expiration
 * attributes are assigned.
 */
void CacheXmlParser::endEntryTimeToLive() {
  if (!flagExpirationAttribute_) {
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
  flagExpirationAttribute_ = false;
}

/**
 * When a <code>entry-idle-time</code> element is finished, the
 * {@link ExpirationAttributes} are on top of the _stack followed by
 * the {@link RegionAttributesFactory} to which the expiration
 * attributes are assigned.
 */
void CacheXmlParser::endEntryIdleTime() {
  if (!flagExpirationAttribute_) {
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
  flagExpirationAttribute_ = false;
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
  if (config_ != nullptr) {
    regionAttributesFactory->setPersistenceManager(
        libraryName->c_str(), libraryFunctionName->c_str(), config_);
    config_ = nullptr;
  } else {
    regionAttributesFactory->setPersistenceManager(
        libraryName->c_str(), libraryFunctionName->c_str());
  }
}

CacheXmlParser::~CacheXmlParser() { _GEODE_SAFE_DELETE(cacheCreation_); }

}  // namespace client
}  // namespace geode
}  // namespace apache
