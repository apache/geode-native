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
 using System;
using System.Text;
using Xunit;
using Apache.Geode.Client;
using System.Linq;
using Microsoft.Extensions.Caching.Distributed;
using System.Threading.Tasks;

namespace Apache.Geode.Session.IntegrationTests
{
    public class SessionStateIntegrationTests
    {
        private static string _regionName = "exampleRegion";

        [Fact]
        public void SetGet()
        {
            var ssCacheOptions = new GeodeSessionStateCacheOptions();
            ssCacheOptions.Host = "localhost";
            ssCacheOptions.Port = 10334;
            ssCacheOptions.RegionName = "exampleRegion";

            using var ssCache = new GeodeSessionStateCache(ssCacheOptions);

            var options = new DistributedCacheEntryOptions();
            DateTime localTime = DateTime.Now.AddDays(1);
            DateTimeOffset dateAndOffset = new DateTimeOffset(localTime,
                           TimeZoneInfo.Local.GetUtcOffset(localTime));
            options.AbsoluteExpiration = dateAndOffset;
            var testValue = new byte[] { 1, 2, 3, 4, 5 };
            ssCache.Set("testKey", testValue, options);
            byte[] value = ssCache.Get("testKey");
            Assert.True(testValue.SequenceEqual(value));
        }

        [Fact]
        public void Refresh()
        {
            var ssCacheOptions = new GeodeSessionStateCacheOptions();
            ssCacheOptions.Host = "localhost";
            ssCacheOptions.Port = 10334;
            ssCacheOptions.RegionName = "exampleRegion";

            using var ssCache = new GeodeSessionStateCache(ssCacheOptions);

            var options = new DistributedCacheEntryOptions();
            int numSeconds = 20;
            options.SlidingExpiration = new TimeSpan(0, 0, numSeconds);
            var testValue = new byte[] { 1, 2, 3, 4, 5 };

            // Set a value
            ssCache.Set("testKey", testValue, options);

            // Wait half a timeout then refresh
            System.Threading.Thread.Sleep(numSeconds / 2 * 1000);
            ssCache.Refresh("testKey");

            // Wait beyond the original expiration
            System.Threading.Thread.Sleep(numSeconds / 2 * 1000 + 1);

            // Ensure it's not expired
            byte[] value = ssCache.Get("testKey");
            Assert.True(testValue.SequenceEqual(value));
        }

        [Fact]
        public void SetWithAbsoluteExpiration()
        {
            var ssCacheOptions = new GeodeSessionStateCacheOptions();
            ssCacheOptions.Host = "localhost";
            ssCacheOptions.Port = 10334;
            ssCacheOptions.RegionName = "exampleRegion";

            using var ssCache = new GeodeSessionStateCache(ssCacheOptions);

            var options = new DistributedCacheEntryOptions();
            options.AbsoluteExpiration = DateTime.Now.AddSeconds(5);
            ssCache.Set("testKey", Encoding.UTF8.GetBytes("testValue"), options);
            System.Threading.Thread.Sleep(6000);
            byte[] value = ssCache.Get("testKey");
            Assert.Null(value);
        }

        [Fact]
        public void Remove()
        {
            var ssCacheOptions = new GeodeSessionStateCacheOptions();
            ssCacheOptions.Host = "localhost";
            ssCacheOptions.Port = 10334;
            ssCacheOptions.RegionName = "exampleRegion";

            using var ssCache = new GeodeSessionStateCache(ssCacheOptions);

            var options = new DistributedCacheEntryOptions();
            DateTime localTime = DateTime.Now.AddDays(1);
            DateTimeOffset dateAndOffset = new DateTimeOffset(localTime,
            TimeZoneInfo.Local.GetUtcOffset(localTime));
            options.AbsoluteExpiration = dateAndOffset;
            var testValue = new byte[] { 1, 2, 3, 4, 5 };
            ssCache.Set("testKey", testValue, options);
            byte[] value = ssCache.Get("testKey");

            ssCache.Remove("testKey");
            value = ssCache.Get("testKey");
            Assert.Null(value);
        }

        [Fact]
        public void SetGetRemoveAsync()
        {
            var ssCacheOptions = new GeodeSessionStateCacheOptions();
            ssCacheOptions.Host = "localhost";
            ssCacheOptions.Port = 10334;
            ssCacheOptions.RegionName = "exampleRegion";

            using var ssCache = new GeodeSessionStateCache(ssCacheOptions);

            var options = new DistributedCacheEntryOptions();
            DateTime localTime = DateTime.Now.AddDays(1);
            DateTimeOffset dateAndOffset = new DateTimeOffset(localTime,
            TimeZoneInfo.Local.GetUtcOffset(localTime));
            options.AbsoluteExpiration = dateAndOffset;

            var testValue1 = new byte[] { 1, 2, 3, 4, 5 };
            var testValue2 = new byte[] { 11, 12, 13, 14, 15 };
            var testValue3 = new byte[] { 21, 22, 23, 24, 25 };
            var testValue4 = new byte[] { 31, 32, 33, 34, 35 };
            var testValue5 = new byte[] { 41, 42, 43, 44, 45 };

            Task set1 = ssCache.SetAsync("testKey1", testValue1, options);
            Task set2 = ssCache.SetAsync("testKey2", testValue2, options);
            Task set3 = ssCache.SetAsync("testKey3", testValue3, options);
            Task set4 = ssCache.SetAsync("testKey4", testValue4, options);
            Task set5 = ssCache.SetAsync("testKey5", testValue5, options);

            Task.WaitAll(set1, set2, set3, set4, set5);

            Task<byte[]> value1 = ssCache.GetAsync("testKey1");
            Task<byte[]> value2 = ssCache.GetAsync("testKey2");
            Task<byte[]> value3 = ssCache.GetAsync("testKey3");
            Task<byte[]> value4 = ssCache.GetAsync("testKey4");
            Task<byte[]> value5 = ssCache.GetAsync("testKey5");

            Task.WaitAll(value1, value2, value3, value4, value5);

            Assert.True(testValue1.SequenceEqual(value1.Result));
            Assert.True(testValue2.SequenceEqual(value2.Result));
            Assert.True(testValue3.SequenceEqual(value3.Result));
            Assert.True(testValue4.SequenceEqual(value4.Result));
            Assert.True(testValue5.SequenceEqual(value5.Result));

            Task rm1 = ssCache.RemoveAsync("testKey1");
            Task rm2 = ssCache.RemoveAsync("testKey2");
            Task rm3 = ssCache.RemoveAsync("testKey3");
            Task rm4 = ssCache.RemoveAsync("testKey4");
            Task rm5 = ssCache.RemoveAsync("testKey5");

            Task.WaitAll(rm1, rm2, rm3, rm4, rm5);

            Assert.Null(ssCache.Get("testKey1"));
            Assert.Null(ssCache.Get("testKey2"));
            Assert.Null(ssCache.Get("testKey3"));
            Assert.Null(ssCache.Get("testKey4"));
            Assert.Null(ssCache.Get("testKey5"));
        }
    }
}
