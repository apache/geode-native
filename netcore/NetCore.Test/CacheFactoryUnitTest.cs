using System;
using Apache.Geode.NetCore;
using Xunit;

namespace GemfireDotNetTest
{
    [Collection("Geode .net Core Collection")]
    public class CacheFactoryUnitTests
    {
        [Fact]
        public void TestCreateFactory()
        {
            using (var cacheFactory = CacheFactory.Create())
            {
                Assert.NotNull(cacheFactory);
            }
        }
        
        [Fact]
        public void TestCacheFactoryGetVersion()
        {
            using (var cacheFactory = CacheFactory.Create())
            {
                var version = cacheFactory.Version;
                Assert.NotEqual(version, String.Empty);
            }
        }
        
        [Fact]
        public void TestCacheFactoryGetProductDescription()
        {
            using (var cacheFactory = CacheFactory.Create())
            {
                var description = cacheFactory.ProductDescription;
                Assert.NotEqual(description, String.Empty);
            }
        }
        
        [Fact]
        public void TestCacheFactorySetPdxIgnoreUnreadFields()
        {
            using (var cacheFactory = CacheFactory.Create())
            {
                cacheFactory.PdxIgnoreUnreadFields = true;
                cacheFactory.PdxIgnoreUnreadFields = false;
            }
        }
        
        [Fact]
        public void TestCacheFactorySetPdxReadSerialized()
        {
            using (var cacheFactory = CacheFactory.Create())
            {
                cacheFactory.PdxReadSerialized = true;
                cacheFactory.PdxReadSerialized = false;
            }
        }
        
        [Fact]
        public void TestCacheFactoryCreateCache()
        {
            using (var cacheFactory = CacheFactory.Create())
            {
                using (var cache = cacheFactory.CreateCache())
                {
                    ;
                }
            }
        }
        
        [Fact]
        public void TestCacheFactorySetProperty()
        {
            using (var cacheFactory = CacheFactory.Create())
            {
                cacheFactory.SetProperty("log-level", "none")
                    .SetProperty("log-file", "geode_native.log");
            }
        }
    }
}