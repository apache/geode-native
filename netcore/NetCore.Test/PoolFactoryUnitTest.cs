using System.Net.Cache;
using Apache.Geode.NetCore;
using Xunit;

namespace GemfireDotNetTest
{
    [Collection("Geode .net Core Collection")]
    public class PoolFactoryUnitTests
    {
        [Fact]
        public void TestPoolFactoryAddLocator()
        {
            using var cacheFactory = CacheFactory.Create()
                    .SetProperty("log-level", "none")
                    .SetProperty("log-file", "geode_native.log");
            using var cache = cacheFactory.CreateCache();
            using var poolManager = cache.PoolManager;
            using var poolFactory = poolManager.CreatePoolFactory();

            poolFactory.AddLocator("localhost", 10334);
        }
        
        [Fact]
        public void TestPoolFactoryCreatePool()
        {
            using var cacheFactory = CacheFactory.Create()
                    .SetProperty("log-level", "none")
                    .SetProperty("log-file", "geode_native.log");
            using var cache = cacheFactory.CreateCache();
            using var poolManager = cache.PoolManager;
            using var poolFactory = poolManager.CreatePoolFactory();

            poolFactory.AddLocator("localhost", 10334);
            using var pool = poolFactory.CreatePool("myPool");
        }

        [Fact]
        public void TestCreatePoolWithoutPoolManager()
        {
            using var cacheFactory = CacheFactory.Create();
            using var cache = cacheFactory.CreateCache();
            using var poolFactory = cache.PoolFactory;
            
            poolFactory.AddLocator("localhost", 10334);
            using var pool = poolFactory.CreatePool("myPool");
        }
    }
}

