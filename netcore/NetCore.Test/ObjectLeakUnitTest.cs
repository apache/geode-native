using System;
using Apache.Geode.NetCore;
using Xunit;

namespace GemfireDotNetTest
{
    [Collection("Geode .net Core Collection")]
    public class ObjectLeakUnitTests
    {
        [Fact]
        public void TestLeakCacheFactory()
        {
            var client = new Client();
            
            using (var cacheFactory = CacheFactory.Create())
            {
                Assert.Throws<InvalidOperationException>(() => client.Dispose());
            }
        }
    }
}