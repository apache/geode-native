using System;
using Microsoft.Extensions.Caching.Distributed;
using Microsoft.Extensions.DependencyInjection;

namespace Apache.Geode.Session
{
    public static class GeodeCacheServiceCollectionExtensions
    {
        public static IServiceCollection AddGeodeSessionStateCache(this IServiceCollection services, Action<GeodeSessionStateCacheOptions> setupAction)
        {
            if (services == null)
            {
                throw new ArgumentNullException(nameof(services));
            }

            if (setupAction == null)
            {
                throw new ArgumentNullException(nameof(setupAction));
            }

            services.AddOptions();
            services.Add(ServiceDescriptor.Singleton<IDistributedCache, GeodeSessionStateCache>());
            services.Configure(setupAction);

            return services;
        }
    }
}
