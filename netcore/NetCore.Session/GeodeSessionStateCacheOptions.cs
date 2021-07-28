using Microsoft.Extensions.Options;

namespace Apache.Geode.Session
{
    public class GeodeSessionStateCacheOptions : IOptions<GeodeSessionStateCacheOptions>
    {
        public string RegionName { get; set; }
        public string Host { get; set; }
        public int Port { get; set; }

        GeodeSessionStateCacheOptions IOptions<GeodeSessionStateCacheOptions>.Value
        {
            get { return this; }
        }
    }
}
