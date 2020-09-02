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

#include <CacheXmlParser.hpp>

#include <gtest/gtest.h>

using apache::geode::client::CacheXmlParser;

static const std::string kXsdPrefix = R"(<?xml version='1.0' encoding='UTF-8'?>
<client-cache
  xmlns="http://geode.apache.org/schema/cpp-cache"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://geode.apache.org/schema/cpp-cache
                      http//geode.apache.org/schema/cpp-cache/cpp-cache-1.0.xsd"
  version='1.0'
>)";

static const std::string kValidCacheConfigBody = R"(<region name = 'Root1' >
        <region-attributes scope='local'
                           caching-enabled='true'
                           initial-capacity='25'
                           load-factor='0.32'
                           concurrency-level='10'
                           lru-entries-limit = '35'>
            <region-idle-time>
                <expiration-attributes timeout='20s' action='destroy'/>
            </region-idle-time>
            <entry-idle-time>
                <expiration-attributes timeout='10s' action='invalidate'/>
            </entry-idle-time>
            <region-time-to-live>
                <expiration-attributes timeout='0s' action='local-destroy'/>
            </region-time-to-live>
            <entry-time-to-live>
                <expiration-attributes timeout='0s' action='local-invalidate'/>
            </entry-time-to-live>
        </region-attributes>
        <region name='SubRegion1'>
            <region-attributes scope='local'
                               caching-enabled='true'
                               initial-capacity='23'
                               load-factor='0.89'
                               concurrency-level='52'>
            </region-attributes>
        </region>
    </region>
    <region name= 'Root2'>
        <region-attributes scope='local'
                           caching-enabled='true'
                           initial-capacity='16'
                           load-factor='0.75'
                           concurrency-level='16'>
            <region-time-to-live>
                <expiration-attributes timeout='0s' action='destroy'/>
            </region-time-to-live>
            <region-idle-time>
                <expiration-attributes timeout='0s' action='invalidate'/>
            </region-idle-time>
            <entry-time-to-live>
                <expiration-attributes timeout='0s' action='destroy'/>
            </entry-time-to-live>
            <entry-idle-time>
                <expiration-attributes timeout='0s' action='invalidate'/>
            </entry-idle-time>
        </region-attributes>
        <region name='SubRegion21'>
            <region-attributes scope='local'
                               caching-enabled='true'
                               initial-capacity='16'
                               load-factor='0.75'
                               concurrency-level='16'>
                <region-idle-time>
                    <expiration-attributes timeout='20s' action='destroy'/>
                </region-idle-time>
                <entry-idle-time>
                    <expiration-attributes timeout='10s' action='invalidate'/>
                </entry-idle-time>
            </region-attributes>
        </region>
        <region name='SubRegion22'>
            <region name='SubSubRegion221'>
            </region>
        </region>
    </region>
</client-cache>)";

static const std::string kInvalidCacheConfigBody = R"(<region >
        <region-attributes scope='local'
                           caching-enabled='true'
                           initial-capacity='25'
                           load-factor='0.32'
                           concurrency-level='10'
                           lru-entries-limit = '35'>
            <region-idle-time>
                <expiration-attributes timeout='20s' action='destroy'/>
            </region-idle-time>
            <entry-idle-time>
                <expiration-attributes timeout='10s' action='invalidate'/>
            </entry-idle-time>
            <region-time-to-live>
                <expiration-attributes timeout='0s' action='local-destroy'/>
            </region-time-to-live>
            <entry-time-to-live>
                <expiration-attributes timeout='0s' action='local-invalidate'/>
            </entry-time-to-live>
        </region-attributes>
        <region name='SubRegion1'>
            <region-attributes scope='local'
                               caching-enabled='true'
                               initial-capacity='23'
                               load-factor='0.89'
                               concurrency-level='52'>
            </region-attributes>
        </region>
    </region>
    <region name= 'Root2'>
        <region-attributes scope='local'
                           caching-enabled='true'
                           initial-capacity='16'
                           load-factor='0.75'
                           concurrency-level='16'>
            <region-time-to-live>
                <expiration-attributes timeout='0s' action='destroy'/>
            </region-time-to-live>
            <region-idle-time>
                <expiration-attributes timeout='0s' action='invalidate'/>
            </region-idle-time>
            <entry-time-to-live>
                <expiration-attributes timeout='0s' action='destroy'/>
            </entry-time-to-live>
            <entry-idle-time>
                <expiration-attributes timeout='0s' action='invalidate'/>
            </entry-idle-time>
        </region-attributes>
        <region name='SubRegion21'>
            <region-attributes scope='local'
                               caching-enabled='true'
                               initial-capacity='16'
                               load-factor='0.75'
                               concurrency-level='16'>
                <region-idle-time>
                    <expiration-attributes timeout='20s' action='destroy'/>
                </region-idle-time>
                <entry-idle-time>
                    <expiration-attributes timeout='10s' action='invalidate'/>
                </entry-idle-time>
            </region-attributes>
        </region>
        <region name='SubRegion22'>
            <region name='SubSubRegion221'>
            </region>
        </region>
    </region>
</client-cache>)";

TEST(CacheXmlParser, CanParseRegionConfigFromAValidXsdCacheConfig) {
  CacheXmlParser parser(nullptr);
  std::string xml = kXsdPrefix + kValidCacheConfigBody;
  parser.parseMemory(xml.c_str(), static_cast<int>(xml.length()));
}

TEST(CacheXmlParser, ParseRegionConfigFromInvalidCacheConfigThrowsException) {
  CacheXmlParser parser(nullptr);
  std::string xml = kXsdPrefix + kInvalidCacheConfigBody;
  ASSERT_THROW(parser.parseMemory(xml.c_str(), static_cast<int>(xml.length())),
               apache::geode::client::CacheXmlException);
}
