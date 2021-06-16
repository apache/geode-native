using System;

namespace Apache
{
    namespace Geode
    {
        namespace NetCore
        {
            public interface IGeodeCache : IRegionService, IDisposable
            {
                bool GetPdxIgnoreUnreadFields();
                bool GetPdxReadSerialized();
//                void InitializeDeclarativeCache(String cacheXml);

//                CacheTransactionManager CacheTransactionManager { get; }
//                DistributedSystem DistributedSystem { get; }
                String Name { get; }
                PoolManager PoolManager { get; }
                PoolFactory PoolFactory { get; }
                void Close();
                void Close(bool keepalive);
                bool Closed { get; }
            }
        }
    }
}