using System;
using System.Net.Cache;
using Apache.Geode.NetCore;
using Xunit;

namespace GemfireDotNetTest
{
    [Collection("Geode .net Core Collection")]
    public class CacheUnitTests
    {
        [Fact]
        public void TestClientCacheGetPdxReadSerialized()
        {
            using var cacheFactory = CacheFactory.Create()
                .SetProperty("log-level", "debug")
                .SetProperty("log-file", "TestClientCacheGetPdxReadSerialized.log");
            try
            {
                cacheFactory.PdxReadSerialized = true;
                using var cache = cacheFactory.CreateCache();

                Assert.True(cache.GetPdxReadSerialized());

                cacheFactory.PdxReadSerialized = false;
                using var otherCache = cacheFactory.CreateCache();

                Assert.False(otherCache.GetPdxReadSerialized());
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                throw;
            }
        }
        
        [Fact]
        public void TestClientCacheGetPdxIgnoreUnreadFields()
        {
            using var cacheFactory = CacheFactory.Create()
                .SetProperty("log-level", "none")
                .SetProperty("log-file", "geode_native.log");
            cacheFactory.PdxIgnoreUnreadFields = true;
            using var cache = cacheFactory.CreateCache();

            Assert.True(cache.GetPdxIgnoreUnreadFields());

            cacheFactory.PdxIgnoreUnreadFields = false;
            using var otherCache = cacheFactory.CreateCache();
            Assert.False(otherCache.GetPdxIgnoreUnreadFields());
        }

        [Fact]
        public void TestClientCacheGetPoolManager()
        {
            using var cacheFactory = CacheFactory.Create()
                .SetProperty("log-level", "none")
                .SetProperty("log-file", "geode_native.log");

            cacheFactory.PdxIgnoreUnreadFields = true;
            using var cache = cacheFactory.CreateCache();
            using var poolManager = cache.PoolManager;
        }
        
        [Fact]
        public void TestClientCacheCreateRegionFactory()
        {
            using var cacheFactory = CacheFactory.Create()
                .SetProperty("log-level", "none")
                .SetProperty("log-file", "geode_native.log");
            cacheFactory.PdxIgnoreUnreadFields = true;
            using var cache = cacheFactory.CreateCache();
            using var regionFactory = cache.CreateRegionFactory(RegionShortcut.Proxy);
        }

        [Fact]
        public void TestClientCacheGetName()
        {
            using var cacheFactory = CacheFactory.Create()
                .SetProperty("log-level", "none");
            cacheFactory.PdxIgnoreUnreadFields = true;
            using var cache = cacheFactory.CreateCache();

            var cacheName = cache.Name;
            Assert.NotEqual(cacheName, String.Empty);
        }
        
        [Fact]
        public void TestClientCacheClose()
        {
            using var cacheFactory = CacheFactory.Create()
                .SetProperty("log-level", "none");
            cacheFactory.PdxIgnoreUnreadFields = true;
            using var cache = cacheFactory.CreateCache();

            Assert.False(cache.Closed);
            cache.Close();
            Assert.True(cache.Closed);
        }
        
        [Fact]
        public void TestClientCacheCloseWithKeepalive()
        {
            using var cacheFactory = CacheFactory.Create()
                .SetProperty("log-level", "none");
            cacheFactory.PdxIgnoreUnreadFields = true;
            using var cache = cacheFactory.CreateCache();
            
            Assert.False(cache.Closed);
            cache.Close(true);
            Assert.True(cache.Closed);

            using var otherCache = cacheFactory.CreateCache();

            Assert.False(otherCache.Closed);
            otherCache.Close(false);
            Assert.True(otherCache.Closed);
        }
    }
}

