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

using System;
using System.IO;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
  [Trait("Category", "Integration")]
  public class RegionSSLTest : IDisposable
  {
    private readonly Cache _cacheOne;
    private readonly GeodeServer _geodeServer;

    public RegionSSLTest()
    {
      var pathvar = Environment.GetEnvironmentVariable("PATH");

      var openSslPath = Environment.CurrentDirectory + Config.OpenSSLPath;

      if (!Directory.Exists(openSslPath))
      {
        throw new DirectoryNotFoundException("OpenSSL is a prerequisite for integration tests and the directory was not found.");
      }

      pathvar += ";" + openSslPath;

      var cryptoImplPath = Environment.CurrentDirectory + Config.CryptoImplPath;

      if (!File.Exists(cryptoImplPath + "\\cryptoImpl.dll"))
      {
        throw new System.IO.FileNotFoundException("cryptoImpl.dll was not found at " + cryptoImplPath);
      }

      pathvar += ";" + cryptoImplPath;

      Environment.SetEnvironmentVariable("PATH", pathvar);

      var cacheFactory = new CacheFactory();
      cacheFactory.Set("ssl-enabled", "true");
      cacheFactory.Set("ssl-keystore", Environment.CurrentDirectory + "\\ClientSslKeys\\client_keystore.password.pem");
      cacheFactory.Set("ssl-keystore-password", "gemstone" );
      cacheFactory.Set("ssl-truststore", Environment.CurrentDirectory + "\\ClientSslKeys\\client_truststore.pem");

      _cacheOne = cacheFactory.Create();
      _geodeServer = new GeodeServer(useSsl: true);
    }

    public void Dispose()
    {
      _cacheOne.Close();
      _geodeServer.Dispose();
    }

    [Fact]
    public void PutGet_Works()
    {
      using (var cacheXml = new CacheXml(new FileInfo("cache.xml"), _geodeServer))
      {
        _cacheOne.InitializeDeclarativeCache(cacheXml.File.FullName);

        var regionForCache1 = _cacheOne.GetRegion<string, string>("testRegion1");

        const string key = "hello";
        const string expectedResult = "dave";

        regionForCache1.Put(key, expectedResult);
        var actualResult = regionForCache1.Get(key);

        Assert.Equal(expectedResult, actualResult);
      }
    }
  }
}