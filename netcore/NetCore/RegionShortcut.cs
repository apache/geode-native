namespace Apache
{
    namespace Geode
    {
        namespace NetCore
        {
            public enum RegionShortcut
            {
                Proxy = 0,
                CachingProxy = 1,
                CachingProxyEntryLru = 2,
                Local = 3,
                LocalEntryLru = 4
            }
        }
    }
}