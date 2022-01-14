/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using Apache.Geode.Client;
using Microsoft.Extensions.Caching.Distributed;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace Apache.Geode.Session {
  public class GeodeSessionStateValue {
    public GeodeSessionStateValue() {}
    public GeodeSessionStateValue(byte[] value) {
      FromByteArray(value);
    }

    public byte[] Value { get; set; }
    public DateTime LastAccessTimeUtc { get; set; }
    public DateTime ExpirationTimeUtc { get; set; } = DateTime.MinValue;
    public TimeSpan SpanUntilStale { get; set; } = TimeSpan.Zero;

    public byte[] ToByteArray() {
      var neededBytes = 3 * sizeof(long) + Value.Length;
      var byteArray = new byte[neededBytes];
      var byteIndex = 0;

      Array.Copy(BitConverter.GetBytes(LastAccessTimeUtc.Ticks), 0, byteArray, byteIndex,
                 sizeof(long));
      byteIndex += sizeof(long);

      Array.Copy(BitConverter.GetBytes(ExpirationTimeUtc.Ticks), 0, byteArray, byteIndex,
                 sizeof(long));
      byteIndex += sizeof(long);

      Array.Copy(BitConverter.GetBytes(SpanUntilStale.Ticks), 0, byteArray, byteIndex,
                 sizeof(long));
      byteIndex += sizeof(long);

      Array.Copy(Value, 0, byteArray, byteIndex, Value.Length);
      return byteArray;
    }

    public void FromByteArray(byte[] data) {
      var byteIndex = 0;

      LastAccessTimeUtc = DateTime.FromBinary(BitConverter.ToInt64(data, byteIndex));
      byteIndex += sizeof(long);

      ExpirationTimeUtc = DateTime.FromBinary(BitConverter.ToInt64(data, byteIndex));
      byteIndex += sizeof(long);

      SpanUntilStale = TimeSpan.FromTicks(BitConverter.ToInt64(data, byteIndex));
      byteIndex += sizeof(long);

      Value = new byte[data.Length - byteIndex];
      Array.Copy(data, byteIndex, Value, 0, data.Length - byteIndex);
    }
  }

  public class GeodeSessionStateCache : GeodeNativeObject, IDistributedCache {
    private readonly IGeodeCache<string, object> _cache;
    private static IRegion<string, object> _region;
    private string _logLevel;
    private string _logFile;
    private string _regionName;
    private readonly SemaphoreSlim _connectLock = new SemaphoreSlim(initialCount: 1, maxCount: 1);

    public GeodeSessionStateCache(IOptions<GeodeSessionStateCacheOptions> optionsAccessor) {
      var host = optionsAccessor.Value.Host;
      var port = optionsAccessor.Value.Port;
      _regionName = optionsAccessor.Value.RegionName;
      _logLevel = optionsAccessor.Value.LogLevel;
      _logFile = optionsAccessor.Value.LogFile;

      _cache = CacheFactory<string, object>.Create()
                   .SetProperty("log-level", _logLevel)
                   .SetProperty("log-file", _logFile)
                   .CreateCache();

      _cache.PoolManager.CreatePoolFactory().AddLocator(host, port).CreatePool("pool");

      var regionFactory = _cache.CreateRegionFactory(RegionShortcut.Proxy);
      _region = regionFactory.CreateRegion(_regionName);
    }

    // Returns the SessionStateValue for key, or null if key doesn't exist
    public GeodeSessionStateValue GetValueForKey(string key) {
      var cacheValue = _region.GetByteArray(key);

      if (cacheValue != null) {
        return new GeodeSessionStateValue(cacheValue);
      } else
        return null;
    }

    public byte[] Get(string key) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      Connect();

      // Check for nonexistent key
      var ssValue = GetValueForKey(key);
      if (ssValue == null) {
        return null;
      }

      // Check for expired key
      var nowUtc = DateTime.UtcNow;
      if (ssValue.ExpirationTimeUtc != DateTime.MinValue && ssValue.ExpirationTimeUtc < nowUtc) {
        return null;
      }

      // Check for stale key
      if (ssValue.SpanUntilStale != TimeSpan.Zero &&
          nowUtc > (ssValue.LastAccessTimeUtc + ssValue.SpanUntilStale)) {
        return null;
      }

      // Update the times for sliding expirations
      if (ssValue.SpanUntilStale != TimeSpan.Zero) {
        ssValue.LastAccessTimeUtc = nowUtc;
        _region.PutByteArray(key, ssValue.ToByteArray());
      }

      return ssValue.Value;
    }

    public Task<byte[]> GetAsync(string key, CancellationToken token = default(CancellationToken)) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      token.ThrowIfCancellationRequested();

      return Task.Factory.StartNew(() => Get(key), token);
    }

    public void Refresh(string key) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      Connect();

      // Check for nonexistent key
      var ssValue = GetValueForKey(key);
      if (ssValue == null) {
        return;
      }

      // Check for expired key
      var nowUtc = DateTime.UtcNow;
      if (ssValue.ExpirationTimeUtc != DateTime.MinValue && ssValue.ExpirationTimeUtc < nowUtc) {
        return;
      }

      // Check for stale key
      if (ssValue.SpanUntilStale != TimeSpan.Zero &&
          nowUtc > (ssValue.LastAccessTimeUtc + ssValue.SpanUntilStale)) {
        return;
      }

      // Update the times for sliding expirations
      if (ssValue.SpanUntilStale != TimeSpan.Zero) {
        ssValue.LastAccessTimeUtc = nowUtc;
        _region.PutByteArray(key, ssValue.ToByteArray());
      }
    }

    public Task RefreshAsync(string key, CancellationToken token = default(CancellationToken)) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      token.ThrowIfCancellationRequested();

      return Task.Factory.StartNew(() => Refresh(key), token);
    }

    public void Remove(string key) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      Connect();

      _region.Remove(key);
    }

    public Task RemoveAsync(string key, CancellationToken token = default(CancellationToken)) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      token.ThrowIfCancellationRequested();

      return Task.Factory.StartNew(() => Remove(key), token);
    }

    public void Set(string key, byte[] value, DistributedCacheEntryOptions options) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      if (value == null) {
        throw new ArgumentNullException(nameof(value));
      }

      if (options == null) {
        throw new ArgumentNullException(nameof(options));
      }

      Connect();

      var ssValue = new GeodeSessionStateValue();
      ssValue.Value = value;

      var nowUtc = DateTime.UtcNow;
      ssValue.LastAccessTimeUtc = nowUtc;

      // No need to check stale or expired data when setting an absolute expiration.
      // Think of if as setting a new key/value pair. Expired data will always be cleaned up
      // when the CleanupExpiredData job runs.

      if (options.AbsoluteExpiration != null) {
        var dto = options.AbsoluteExpiration.Value;
        ssValue.ExpirationTimeUtc = dto.DateTime + dto.Offset;
      }

      // If AbsoluteExpiration and AbsoluteExpirationRelativeToNow are set, use the latter.
      if (options.AbsoluteExpirationRelativeToNow != null) {
        var ts = options.AbsoluteExpirationRelativeToNow.Value;
        ssValue.ExpirationTimeUtc = nowUtc + ts;
      }

      if (options.SlidingExpiration != null) {
        ssValue.SpanUntilStale = options.SlidingExpiration.Value;
      }

      _region.PutByteArray(key, ssValue.ToByteArray());
      return;
    }

    public Task SetAsync(string key, byte[] value, DistributedCacheEntryOptions options,
                         CancellationToken token = default(CancellationToken)) {
      if (key == null) {
        throw new ArgumentNullException(nameof(key));
      }

      token.ThrowIfCancellationRequested();

      return Task.Factory.StartNew(() => Set(key, value, options), token);
    }

    private void Connect() {
      if (_region != null) {
        return;
      }

      _connectLock.Wait();
      using var regionFactory = _cache.CreateRegionFactory(RegionShortcut.Proxy);
      _region = regionFactory.CreateRegion(_regionName);
      _connectLock.Release();
    }

    protected override void DestroyContainedObject() {
      _region?.Dispose();
      _region = null;
    }
  }
}
