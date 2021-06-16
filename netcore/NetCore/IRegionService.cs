using System;

namespace Apache
{
    namespace Geode
    {
        namespace NetCore
        {
            public interface IRegionService
            {
//                IPdxInstanceFactory CreatePdxInstanceFactory(String className);
//                QueryService<TKey, TResult> GetQueryService();
//                IRegion<TKey, TValue> GetRegion(String name);
//                Array<IRegion<TKey, TValue> RootRegions();
                RegionFactory CreateRegionFactory(RegionShortcut regionType);
            }
        }
    }
}