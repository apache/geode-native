using Apache.Geode.Client;
using Microsoft.Extensions.Caching.Distributed;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace Apache.Geode.Session
{
    public class GeodeSessionStateValue
    {
        DateTime _lastAccessTimeUtc;
        DateTime _expirationTimeUtc = DateTime.MinValue;
        TimeSpan _spanUntilStale = TimeSpan.Zero;
        private byte[] _value;

        public GeodeSessionStateValue() { }
        public GeodeSessionStateValue(byte[] value)
        {
            FromByteArray(value);
        }

        public byte[] Value
        {
            get { return _value; }
            set { _value = value; }
        }
        public DateTime LastAccessTimeUtc
        {
            get { return _lastAccessTimeUtc; }
            set { _lastAccessTimeUtc = value; }
        }

        public DateTime ExpirationTimeUtc
        {
            get { return _expirationTimeUtc; }
            set { _expirationTimeUtc = value; }
        }

        public TimeSpan SpanUntilStale
        {
            get { return _spanUntilStale; }
            set { _spanUntilStale = value; }
        }

        public byte[] ToByteArray()
        {
            int neededBytes = 3 * sizeof(long) + _value.Length;
            byte[] byteArray = new byte[neededBytes];
            int byteIndex = 0;

            // Append LastAccessTimeUtc
            Array.Copy(BitConverter.GetBytes(LastAccessTimeUtc.Ticks), 0, byteArray, byteIndex, sizeof(long));
            byteIndex += sizeof(long);

            // Append ExpirationTimeUtc
            Array.Copy(BitConverter.GetBytes(ExpirationTimeUtc.Ticks), 0, byteArray, byteIndex, sizeof(long));
            byteIndex += sizeof(long);

            // Append SpanUntilStale
            Array.Copy(BitConverter.GetBytes(SpanUntilStale.Ticks), 0, byteArray, byteIndex, sizeof(long));
            byteIndex += sizeof(long);

            // Append the value
            Array.Copy(_value, 0, byteArray, byteIndex, _value.Length);
            return byteArray;
        }

        public void FromByteArray(byte[] data)
        {
            int byteIndex = 0;

            // Extract the LastAccessTimeUtc
            LastAccessTimeUtc = DateTime.FromBinary(BitConverter.ToInt64(data, byteIndex));
            byteIndex += sizeof(long);

            // Extract the ExpirationTimeUtc
            ExpirationTimeUtc = DateTime.FromBinary(BitConverter.ToInt64(data, byteIndex));
            byteIndex += sizeof(long);

            // Extract the SpanUntilStale
            SpanUntilStale = TimeSpan.FromTicks(BitConverter.ToInt64(data, byteIndex));
            byteIndex += sizeof(long);

            // Extract the value
            Value = new byte[data.Length - byteIndex];
            Array.Copy(data, byteIndex, _value, 0, data.Length - byteIndex);
        }
    }

    public class GeodeSessionStateCache : GeodeNativeObject, IDistributedCache
    {
        private readonly IGeodeCache _cache;
        private ILogger<GeodeSessionStateCache> _logger;
        private static Region _region;
        private string _regionName;
        private readonly SemaphoreSlim _connectLock = new SemaphoreSlim(initialCount: 1, maxCount: 1);

        public GeodeSessionStateCache(IOptions<GeodeSessionStateCacheOptions> optionsAccessor) {

            var host = optionsAccessor.Value.Host;
            var port = optionsAccessor.Value.Port;
            _regionName = optionsAccessor.Value.RegionName;

            _cache = CacheFactory.Create()
                .SetProperty("log-level", "none")
                .CreateCache();

            _cache.PoolManager
                .CreatePoolFactory()
                .AddLocator(host, port)
                .CreatePool("pool");

            var regionFactory = _cache.CreateRegionFactory(RegionShortcut.Proxy);
            _region = regionFactory.CreateRegion(_regionName);
        }

        // Returns the SessionStateValue for key, or null if key doesn't exist
        public GeodeSessionStateValue GetValueForKey(string key)
        {
            byte[] cacheValue = _region.GetByteArray(key);

            if (cacheValue != null)
            {
                return new GeodeSessionStateValue(cacheValue);
            }
            else
                return null;
        }

        public byte[] Get(string key)
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            Connect();

            // Check for nonexistent key
            GeodeSessionStateValue ssValue = GetValueForKey(key);
            if (ssValue == null)
                return null;

            // Check for expired key
            DateTime nowUtc = DateTime.UtcNow;
            if (ssValue.ExpirationTimeUtc != DateTime.MinValue && ssValue.ExpirationTimeUtc < nowUtc)
                return null;

            // Check for stale key
            if (ssValue.SpanUntilStale != TimeSpan.Zero &&
              nowUtc > (ssValue.LastAccessTimeUtc + ssValue.SpanUntilStale))
                return null;

            //LogDebug("Inserting against key [" + key + "] with absolute expiration: " +
            //         options.AbsoluteExpiration.Value.DateTime);

            // Update the times for sliding expirations
            if (ssValue.SpanUntilStale != TimeSpan.Zero)
            {
                ssValue.LastAccessTimeUtc = nowUtc;
                _region.PutByteArray(key, ssValue.ToByteArray());
            }

            return ssValue.Value;
        }

        public Task<byte[]> GetAsync(string key, CancellationToken token = default(CancellationToken))
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            token.ThrowIfCancellationRequested();

            return Task.Factory.StartNew(() => Get(key), token);
        }

        public void Refresh(string key)
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            Connect();

            // Check for nonexistent key
            GeodeSessionStateValue ssValue = GetValueForKey(key);
            if (ssValue == null)
                return;

            // Check for expired key
            DateTime nowUtc = DateTime.UtcNow;
            if (ssValue.ExpirationTimeUtc != DateTime.MinValue && ssValue.ExpirationTimeUtc < nowUtc)
                return;

            // Check for stale key
            if (ssValue.SpanUntilStale != TimeSpan.Zero &&
              nowUtc > (ssValue.LastAccessTimeUtc + ssValue.SpanUntilStale))
                return;

            //LogDebug("Inserting against key [" + key + "] with absolute expiration: " +
            //         options.AbsoluteExpiration.Value.DateTime);

            // Update the times for sliding expirations
            if (ssValue.SpanUntilStale != TimeSpan.Zero)
            {
                ssValue.LastAccessTimeUtc = nowUtc;
                _region.PutByteArray(key, ssValue.ToByteArray());
            }
        }

        public Task RefreshAsync(string key, CancellationToken token = default(CancellationToken))
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            token.ThrowIfCancellationRequested();

            return Task.Factory.StartNew(() => Refresh(key), token);
        }

        public void Remove(string key)
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            Connect();

            // Until we return error codes
            //if (!_cacheRegion.Remove(key))
            //{
            //    throw new Exception("Failed to remove from cache");
            //}
            _region.Remove(key);
        }

        public Task RemoveAsync(string key, CancellationToken token = default(CancellationToken))
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            token.ThrowIfCancellationRequested();

            return Task.Factory.StartNew(() => Remove(key), token);
        }

        public void Set(string key, byte[] value, DistributedCacheEntryOptions options)
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            if (options == null)
            {
                throw new ArgumentNullException(nameof(options));
            }

            Connect();

            GeodeSessionStateValue ssValue = new GeodeSessionStateValue();
            ssValue.Value = value;

            DateTime nowUtc = DateTime.UtcNow;
            ssValue.LastAccessTimeUtc = nowUtc;

            // No need to check stale or expired data when setting an absolute expiration.
            // Think of if as setting a new key/value pair. Expired data will always be cleaned up
            // when the CleanupExpiredData job runs.

            if (options.AbsoluteExpiration != null)
            {
                //LogDebug("Inserting against key [" + key + "] with absolute expiration: " +
                //         options.AbsoluteExpiration.Value.DateTime);
                DateTimeOffset dto = options.AbsoluteExpiration.Value;
                ssValue.ExpirationTimeUtc = dto.DateTime + dto.Offset;
            }

            // If AbsoluteExpiration and AbsoluteExpirationRelativeToNow are set, use the latter.
            if (options.AbsoluteExpirationRelativeToNow != null)
            {
                //LogDebug("Inserting against key [" + key + "] with absolute expiration: " +
                //         options.AbsoluteExpiration.Value.DateTime);
                TimeSpan ts = options.AbsoluteExpirationRelativeToNow.Value;
                ssValue.ExpirationTimeUtc = nowUtc + ts;
            }

            if (options.SlidingExpiration != null)
            {
                //LogDebug("Inserting against key [" + key + "] with absolute expiration: " +
                //         options.AbsoluteExpiration.Value.DateTime);
                ssValue.SpanUntilStale = options.SlidingExpiration.Value;
            }

            _region.PutByteArray(key, ssValue.ToByteArray());
            return;
        }

        public Task SetAsync(string key, byte[] value, DistributedCacheEntryOptions options, CancellationToken token = default(CancellationToken))
        {
            if (key == null)
            {
                throw new ArgumentNullException(nameof(key));
            }

            token.ThrowIfCancellationRequested();

            return Task.Factory.StartNew(() => Set(key, value, options), token);
        }

        private void Connect()
        {
            if (_region != null)
            {
                return;
            }

            _connectLock.Wait();
            try
            {
                using var regionFactory = _cache.CreateRegionFactory(RegionShortcut.Proxy);
                try
                {
                    _logger?.LogTrace("Create CacheRegion");
                    _region = regionFactory.CreateRegion(_regionName);
                    _logger?.LogTrace("CacheRegion created");
                }
                catch (Exception e)
                {
                    _logger?.LogInformation(e, "Create CacheRegion failed... now trying to get the region");
                }
            }
            finally
            {
                //regionFactory?.Dispose();
                _connectLock.Release();
            }
        }

        protected override void DestroyContainedObject()
        {
            _region?.Dispose();
            _region = null;
        }
    }
}