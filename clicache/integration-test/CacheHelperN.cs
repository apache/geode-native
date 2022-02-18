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
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml;

#pragma warning disable 618

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using Apache.Geode.DUnitFramework;
  using Apache.Geode.Client;
  using System.Management;
  using System.Threading;

  public class PropsStringToObject
  {
    public PropsStringToObject(Properties<string, object> target)
    {
      m_target = target;
    }

    public void Visit(string key, string val)
    {
      if (key == "security-signature")
      {
        Util.Log("VJR: Got SIG as " + val);
        string[] stringbytes = val.Split(' ');
        byte[] credentialbytes = new byte[stringbytes.Length - 1];
        int position = 0;
        foreach (string item in stringbytes)
        {
          Util.Log("VJR: Parsing byte " + item);
          if (string.IsNullOrEmpty(item)) continue;
          credentialbytes[position++] = byte.Parse(item);
        }
        m_target.Insert(key, credentialbytes);
      }
      else
      {
        m_target.Insert(key, val);
      }
    }

    private Properties<string, object> m_target;
  }

  public class PutGetTestsAD : MarshalByRefObject
  {
    private static string m_regionName;
    private static PutGetTests m_putGetTestInstance = new PutGetTests();

    public int InitKeys(UInt32 typeId, int numKeys, int maxSize)
    {
      return m_putGetTestInstance.InitKeys(typeId, numKeys, maxSize);
    }

    public void InitValues(UInt32 typeId, int numValues, int maxSize)
    {
      m_putGetTestInstance.InitValues(typeId, numValues, maxSize);
    }

    public void DoPuts()
    {
      m_putGetTestInstance.DoPuts();
    }

    public void DoKeyChecksumPuts()
    {
      m_putGetTestInstance.DoKeyChecksumPuts();
    }

    public void DoValChecksumPuts()
    {
      m_putGetTestInstance.DoValChecksumPuts();
    }

    public void DoGetsVerify()
    {
      m_putGetTestInstance.DoGetsVerify();
    }

    public void InvalidateRegion(string regionName)
    {
      CacheHelper.InvalidateRegion<object, object>(regionName, true, true);
    }

    public void DoRunQuery()
    {
      m_putGetTestInstance.DoRunQuery();
    }

    public void SetRegion(string regionName)
    {
      m_regionName = regionName;
      m_putGetTestInstance.SetRegion(regionName);
    }

    public void DoGets()
    {
      m_putGetTestInstance.DoGets();
    }

    public void pdxPutGet(bool caching, bool readPdxSerialized)
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTests.PdxType.CreateDeserializable);
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>(m_regionName);
      PdxTests.PdxType pt = new PdxTests.PdxType();
      reg["pi"] = pt;

      object pi = null;

      if (caching)
      {
        pi = reg.GetLocalView()["pi"];
      }
      else
      {
        pi = reg["pi"];
      }

      if (readPdxSerialized)
      {
        int iv = (int)((IPdxInstance)pi).GetField("m_int32");
        Assert.AreEqual(iv, pt.Int32);
      }
      else
      {
        Assert.AreEqual(pi, pt);
      }
    }

    public void pdxGetPut(bool caching, bool readPdxSerialized)
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTests.PdxType.CreateDeserializable);
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>(m_regionName);
      PdxTests.PdxType pt = new PdxTests.PdxType();

      object pi = null;

      if (caching)
      {
        pi = reg.GetLocalView()["pi"];
      }
      else
      {
        pi = reg["pi"];
      }

      if (readPdxSerialized)
      {
        int iv = (int)((IPdxInstance)pi).GetField("m_int32") + 10;

        IWritablePdxInstance wpi = ((IPdxInstance)pi).CreateWriter();

        wpi.SetField("m_int32", iv);

        reg["pi"] = wpi;

        if (caching)
        {
          pi = reg.GetLocalView()["pi"];
        }
        else
        {
          pi = reg["pi"];
        }

        iv = (int)((IPdxInstance)pi).GetField("m_int32");

        Assert.AreEqual(iv, pt.Int32 + 10);
      }
      else
      {
        Assert.AreEqual(pi, pt);
      }
    }
  }

  public class CacheHelperWrapper : MarshalByRefObject
  {
    public void CreateTCRegions_Pool_AD<TKey, TValue>(string[] regionNames,
      string locators, string poolName, bool clientNotification, bool ssl, bool caching, bool pdxReadSerialized)
    {
      try
      {
        Console.WriteLine("creating region1 " + pdxReadSerialized);
        CacheHelper.PdxReadSerialized = pdxReadSerialized;
        CacheHelper.CreateTCRegion_Pool_AD<TKey, TValue>(regionNames[0], true, caching,
          null, locators, poolName, clientNotification, ssl, false);
        //CacheHelper.CreateTCRegion_Pool(regionNames[1], false, true,
        //null, endpoints, locators, poolName, clientNotification, ssl, false);
        //     m_regionNames = regionNames;
        CacheHelper.PdxReadSerialized = false;
        Util.Log("created region " + regionNames[0]);
      }
      catch (AlreadyConnectedException)
      {
        Console.WriteLine("Got already connected exception  in TEST");
        Util.Log("Got already connected exception " + regionNames[0]);
      }
    }

    public void CallDistrinbutedConnect()
    {
      try
      {
        //Console.WriteLine(" cakk CallDistrinbutedConnect");
        CacheHelper.DSConnectAD();
      }
      catch (Exception ex)
      {
        //Console.WriteLine(" cakk CallDistrinbutedConnect 33");
        Util.Log(" got AlreadyConnectedException " + ex.Message);
      }
    }

    public void RegisterBuiltins(long dtTime)
    {
      CacheableHelper.RegisterBuiltinsAD(dtTime);
    }

    public void InvalidateRegion(string regionName)
    {
      CacheHelper.InvalidateRegion<object, object>(regionName, true, true);
    }

    public void CloseCache()
    {
      CacheHelper.Close();
    }

    public void SetLogFile(string logFileName)
    {
      Util.LogFile = logFileName;
    }
  }

  /// <summary>
  /// Helper class to create/destroy Distributed System, cache and regions.
  /// This class is intentionally not thread-safe.
  /// </summary>
  public class CacheHelper
  {
    public static string TestDir
    {
      get
      {
        if (m_testDir == null)
        {
          m_testDir = Util.GetEnvironmentVariable("TESTSRC");
          if (m_testDir == null)
          {
            return ".";
          }
        }
        return m_testDir;
      }
    }

    public static int HOST_PORT_1;
    public static int HOST_PORT_2;
    public static int HOST_PORT_3;
    public static int HOST_PORT_4;

    public static bool PdxIgnoreUnreadFields = false;
    public static bool PdxReadSerialized = false;

    public static int LOCATOR_PORT_1;
    public static int LOCATOR_PORT_2;
    public static int LOCATOR_PORT_3;
    public static int LOCATOR_PORT_4;

    public static int JMX_MANAGER_PORT;

    #region Private static members and constants

    private static DistributedSystem m_dsys = null;
    private static bool m_doDisconnect = false;
    private static Cache m_cache = null;
    private static IRegionService m_cacheForMultiUser = null;
    private static IRegion<object, object> m_currRegion = null;
    private static string m_gfeDir = null;
    private static string m_gfeLogLevel = null;
    private static string m_gfeSecLogLevel = null;
    private static string m_endpoints = null;
    private static string m_locators = null;
    private static string m_locatorFirst = null;
    private static string m_locatorSecond = null;
    private static string[] m_cacheXmls = null;
    private static bool m_localServer = true;
    private static string m_extraPropertiesFile = null;

    private static string gfsh = null;
    private const string JavaServerStartArgs = "start server --max-heap=512m --cache-xml-file=";
    private const string JavaServerStopArgs = "stop server";
    private const string LocatorStartArgs = "start locator --max-heap=512m";
    private const string LocatorStopArgs = "stop locator";
    private const int MaxWaitMillis = 60000*5;

    private static string m_testDir = null;
    private static Dictionary<int, string> m_runningJavaServers =
      new Dictionary<int, string>();
    private static Dictionary<int, string> m_runningLocators =
      new Dictionary<int, string>();
    private static CacheTransactionManager m_cstxManager = null;

    private static readonly string tempDirectoryRoot = Path.Combine(Directory.GetCurrentDirectory(), Path.GetRandomFileName());
    #endregion

    #region Public accessors

    public static IRegion<object, object> CurrentRegion
    {
      get
      {
        return m_currRegion;
      }
    }

    public static CacheTransactionManager CSTXManager
    {
      get
      {
        return m_cstxManager;
      }
    }
    public static Cache DCache
    {
      get
      {
        return m_cache;
      }
      set
      {
        m_cache = value;
      }
    }

    public static string Locators
    {
      get
      {
        return m_locators;
      }
    }

    public static string LocatorSecond
    {
      get
      {
        return m_locatorSecond;
      }
    }

    public static string LocatorFirst
    {
      get
      {
        return m_locatorFirst;
      }
    }

    public static string ExtraPropertiesFile
    {
      get
      {
        return m_extraPropertiesFile;
      }
    }

    /*
    public static QueryService QueryServiceInstance
    {
      get
      {
        return m_cache.GetQueryService();
      }
    }
     * */

    public const string DefaultRegionName = "regiontest";

    #endregion

    #region Functions to initialize or close a cache and distributed system

    public static void SetLogging()
    {
      if (Util.LogFile != null)
      {
        string logFile = Regex.Replace(Util.LogFile, "\\....$", string.Empty);
        LogLevel logLevel;
        if (Util.CurrentLogLevel != Util.DefaultLogLevel)
        {
          logLevel = (LogLevel)Util.CurrentLogLevel;
        }
        else
        {
          logLevel = Log.Level();
        }
        Log.Close();
        Log.Init(logLevel, logFile);
      }
    }

    public static void DSConnectAD()
    {
      m_dsys = DistributedSystem.Connect("DSName", null, m_cache);
    }

    private static void SetLogConfig(ref Properties<string, string> config)
    {
      if (Util.LogFile != null)
      {
        Log.Close();
        if (config == null)
        {
          config = new Properties<string, string>();
        }
        if (Util.LogFile != null && Util.LogFile.Length > 0)
        {
          string logFile = Regex.Replace(Util.LogFile, "\\....$", string.Empty);
          config.Insert("log-file", logFile);
        }
        if (Util.CurrentLogLevel != Util.DefaultLogLevel)
        {
          config.Insert("log-level", Util.CurrentLogLevel.ToString().ToLower());
        }
      }
    }

    private static void DSConnect(string dsName, Properties<string, string> config)
    {
      SetLogConfig(ref config);
      m_dsys = DistributedSystem.Connect(dsName, config, m_cache);
    }

    public static void ConnectName(string dsName)
    {
      ConnectConfig(dsName, null);
    }

    public static void ConnectConfig(string dsName, Properties<string, string> config)
    {
      DSConnect(dsName, config);
    }

    public static void Init()
    {
      InitConfig(null, null);
    }

    public static void InitConfig(Properties<string, string> config, IAuthInitialize authInitialize)
    {
      InitConfig(config, null, authInitialize);
    }

    public static void InitConfig(Properties<string, string> config)
    {
      InitConfig(config, null);
    }

    public static void InitConfigForDurable_Pool(string locators, int redundancyLevel,
      string durableClientId, TimeSpan durableTimeout)
    {
      InitConfigForDurable_Pool(locators, redundancyLevel, durableClientId, durableTimeout, TimeSpan.FromSeconds(1));
    }

    public static void InitConfigForDurable_Pool(string locators, int redundancyLevel,
      string durableClientId, TimeSpan durableTimeout, TimeSpan ackInterval)
    {
      Properties<string, string> config = new Properties<string, string>();
      config.Insert("durable-client-id", durableClientId);
      config.Insert("durable-timeout", durableTimeout.TotalSeconds + "s");
      InitConfig(config, null);
      CreatePool<object, object>("__TESTPOOL1_", locators, (string)null, redundancyLevel, true,
        ackInterval, TimeSpan.FromSeconds(300));
    }

    public static void InitConfigForDurable_Pool2(string locators, int redundancyLevel,
      string durableClientId, TimeSpan durableTimeout, TimeSpan ackInterval, string poolName)
    {
      Properties<string, string> config = new Properties<string, string>();
      config.Insert("durable-client-id", durableClientId);
      config.Insert("durable-timeout", durableTimeout.TotalSeconds + "s");
      InitConfig(config, null);
      CreatePool<object, object>(poolName, locators, (string)null, redundancyLevel, true,
        ackInterval, TimeSpan.FromSeconds(300));
    }

    public static void InitConfigForConflation(string durableClientId, string conflation)
    {
      Properties<string, string> config = new Properties<string, string>();
      config.Insert("durable-client-id", durableClientId);
      config.Insert("durable-timeout", "300s");
      config.Insert("notify-ack-interval", "1s");
      if (conflation != null && conflation.Length > 0)
      {
        config.Insert("conflate-events", conflation);
      }
      InitConfig(config, null);
    }

    static int m_heapLimit = -1;
    static int m_delta = -1;
    static public void SetHeapLimit(int maxheaplimit, int delta)
    {
      m_heapLimit = maxheaplimit;
      m_delta = delta;
    }

    static public void UnsetHeapLimit()
    {
      m_heapLimit = -1;
      m_delta = -1;
    }

    public static void InitConfigForConflation_Pool(string locators,
      string durableClientId, string conflation)
    {
      Properties<string, string> config = new Properties<string, string>();
      config.Insert("durable-client-id", durableClientId);
      config.Insert("durable-timeout", "300s");
      config.Insert("notify-ack-interval", "1s");
      if (conflation != null && conflation.Length > 0)
      {
        config.Insert("conflate-events", conflation);
      }
      InitConfig(config, null);
      CreatePool<object, object>("__TESTPOOL1_", locators, (string)null, 0, true);
    }

    public static void InitConfig(string cacheXml)
    {
      InitConfig(null, cacheXml, null);
    }

    public static void InitConfig(Properties<string, string> config, string cacheXml, IAuthInitialize authIntialize)
    {
      if (cacheXml != null)
      {
        var duplicateXMLFile = Path.Combine(MakeTempDirectory(), cacheXml);
        createDuplicateXMLFile(cacheXml, duplicateXMLFile);
        cacheXml = duplicateXMLFile;
      }
      if (config == null)
      {
        config = new Properties<string, string>();
      }
      if (m_extraPropertiesFile != null)
      {
        config.Load(m_extraPropertiesFile);
      }

      if (m_cache == null || m_cache.IsClosed)
      {

        try
        {
          CacheHelper.m_doDisconnect = false;
          config.Insert("enable-time-statistics", "true");
          SetLogConfig(ref config);

          if (m_heapLimit != -1)
            config.Insert("heap-lru-limit", m_heapLimit.ToString());
          if (m_delta != -1)
            config.Insert("heap-lru-delta", m_delta.ToString());
          config.Insert("enable-time-statistics", "true");

          var cf = new CacheFactory(config);

          if (cacheXml != null && cacheXml.Length > 0)
          {
            cf = cf.Set("cache-xml-file", cacheXml);
          }

          m_cache = cf
              .SetPdxIgnoreUnreadFields(PdxIgnoreUnreadFields)
              .SetPdxReadSerialized(PdxReadSerialized)
              .SetAuthInitialize(authIntialize)
              .Create();

          PdxIgnoreUnreadFields = false; //reset so next test will have default value
          PdxReadSerialized = false;
        }
        catch (CacheExistsException)
        {
          m_cache = new CacheFactory(config).Create();
        }
      }

      m_cstxManager = m_cache.CacheTransactionManager;
    }

    public static void SetExtraPropertiesFile(string fName)
    {
      m_extraPropertiesFile = fName;
    }

    public static void InitClient()
    {
      CacheHelper.Close();
      Properties<string, string> config = new Properties<string, string>();
      config.Load("geode.properties");
      CacheHelper.InitConfig(config);
    }

    public static void Close()
    {
      Util.Log("in cache close : " + System.Threading.Thread.GetDomainID());
      //if (DistributedSystem.IsConnected)
      {
        CloseCache();
        if (m_doDisconnect)
        {
          //  DistributedSystem.Disconnect();
        }
      }
      m_dsys = null;
      m_cacheForMultiUser = null;
    }


    public static void CloseUserCache(bool keepAlive)
    {
      //TODO: need to look
      m_cacheForMultiUser.Close();
    }

    public static void CloseCache()
    {
      Util.Log("A CloseCache " + (m_cache != null ? m_cache.IsClosed.ToString() : "cache is closed"));
      if (m_cache != null && !m_cache.IsClosed)
      {
        m_cache.Close();
      }
      m_cache = null;
    }

    public static void CloseKeepAlive()
    {
      CloseCacheKeepAlive();
      m_dsys = null;
    }

    public static void CloseCacheKeepAlive()
    {
      if (m_cache != null && !m_cache.IsClosed)
      {
        m_cache.Close(true);
      }
      m_cache = null;
    }

    public static void ReadyForEvents()
    {
      if (m_cache != null && !m_cache.IsClosed)
      {
        m_cache.ReadyForEvents();
      }
    }

    #endregion

    #region Functions to create or destroy a region

    public static IRegion<TKey, TValue> CreateRegion<TKey, TValue>(string name, Apache.Geode.Client.RegionAttributes<TKey, TValue> attrs)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      //region = m_cache.CreateRegion(name, attrs);
      region = m_cache.CreateRegionFactory(RegionShortcut.LOCAL).Create<TKey, TValue>(name);
      Assert.IsNotNull(region, "IRegion<object, object> was not created.");
      m_currRegion = region as IRegion<object, object>;
      return region;
    }


    public static IRegion<TKey, TValue> CreateExpirationRegion<TKey, TValue>(
      string name, string poolname, ExpirationAction action, TimeSpan entryTimeToLive)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      region = m_cache.CreateRegionFactory(RegionShortcut.CACHING_PROXY)
        .SetEntryTimeToLive(action, entryTimeToLive).SetPoolName(poolname).Create<TKey, TValue>(name);
      Assert.IsNotNull(region, "IRegion<object, object> was not created.");
      m_currRegion = region as IRegion<object, object>;
      return region;
    }

    public static IRegion<TKey, TValue> CreateLocalRegionWithETTL<TKey, TValue>(
      string name, ExpirationAction action, TimeSpan entryTimeToLive)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      region = m_cache.CreateRegionFactory(RegionShortcut.LOCAL)
        .SetEntryTimeToLive(action, entryTimeToLive).Create<TKey, TValue>(name);
      Assert.IsNotNull(region, "IRegion<object, object> was not created.");
      m_currRegion = region as IRegion<object, object>;
      return region;
    }

    public static void CreateDefaultRegion<TKey, TValue>()
    {
      CreatePlainRegion<TKey, TValue>(DefaultRegionName);
    }

    public static IRegion<TKey, TValue> CreatePlainRegion<TKey, TValue>(string name)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      region = m_cache.CreateRegionFactory(RegionShortcut.LOCAL).Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      return region;
    }

    public static void CreateCachingRegion<TKey, TValue>(string name, bool caching)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      region = m_cache.CreateRegionFactory(RegionShortcut.PROXY).SetCachingEnabled(caching).Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
    }

    public static void CreateDistribRegion<TKey, TValue>(string name, bool ack,
      bool caching)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      region = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetCachingEnabled(caching).SetInitialCapacity(100000).Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
    }

    public static IRegion<TKey, TValue> CreateDistRegion<TKey, TValue>(string rootName,
      string name, int size)
    {
      Init();
      CreateCachingRegion<TKey, TValue>(rootName, true);
      var regionAttributesFactory = new RegionAttributesFactory<TKey, TValue>();
      regionAttributesFactory.SetLruEntriesLimit(0);
      regionAttributesFactory.SetInitialCapacity(size);
      var regionAttributes = regionAttributesFactory.Create();
      IRegion<TKey, TValue> region = ((Region<TKey, TValue>)m_currRegion).CreateSubRegion(name, regionAttributes);
      Assert.IsNotNull(region, "SubRegion {0} was not created.", name);
      return region;
    }

    public static IRegion<TKey, TValue> CreateILRegion<TKey, TValue>(string name, bool ack, bool caching,
      ICacheListener<TKey, TValue> listener)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      RegionFactory regionFactory = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetInitialCapacity(100000)
        .SetCachingEnabled(caching);

      if (listener != null)
      {
        regionFactory.SetCacheListener(listener);
      }

      region = regionFactory.Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      return region;
    }

    public static IRegion<TKey, TValue> CreateSizeRegion<TKey, TValue>(string name, int size, bool ack,
      bool caching)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      region = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetLruEntriesLimit(0).SetInitialCapacity(size)
        .SetCachingEnabled(caching).Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      return region;
    }

    public static IRegion<TKey, TValue> CreateLRURegion<TKey, TValue>(string name, uint size)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      region = m_cache.CreateRegionFactory(RegionShortcut.LOCAL_ENTRY_LRU)
        .SetLruEntriesLimit(size).SetInitialCapacity((int)size)
        .Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      return region;
    }


    public static IRegion<TradeKey, Object> CreateTCRegion2<TradeKey, Object>(string name, bool ack, bool caching,
      IPartitionResolver<TradeKey, Object> resolver, string locators, bool clientNotification)
    {
      Init();
      IRegion<TradeKey, Object> region = GetRegion<TradeKey, Object>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      RegionFactory regionFactory = m_cache.CreateRegionFactory(RegionShortcut.CACHING_PROXY);

      regionFactory.SetInitialCapacity(100000);
      regionFactory.SetCachingEnabled(caching);

      if (resolver != null)
      {
        Util.Log("resolver is attached {0}", resolver);
        regionFactory.SetPartitionResolver(resolver);
      }
      else
      {
        Util.Log("resolver is null {0}", resolver);
      }

      PoolFactory poolFactory = m_cache.GetPoolFactory();

      if (locators != null)
      {
        string[] list = locators.Split(',');
        foreach (string item in list)
        {
          string[] parts = item.Split(':');
          poolFactory.AddLocator(parts[0], int.Parse(parts[1]));
          Util.Log("AddLocator parts[0] = {0} int.Parse(parts[1]) = {1} ", parts[0], int.Parse(parts[1]));
        }
      }
      else
      {
        Util.Log("No locators or servers specified for pool");
      }

      poolFactory.SetSubscriptionEnabled(clientNotification);
      poolFactory.Create("__TESTPOOL__");
      region = regionFactory.SetPoolName("__TESTPOOL__").Create<TradeKey, Object>(name);

      Assert.IsNotNull(region, "IRegion<TradeKey, Object> {0} was not created.", name);
      Util.Log("IRegion<TradeKey, Object> {0} has been created with attributes:{1}",
        name, RegionAttributesToString(region.Attributes));
      return region;
    }

    public static Pool/*<TKey, TValue>*/ CreatePool<TKey, TValue>(string name, string locators, string serverGroup,
      int redundancy, bool subscription)
    {
      return CreatePool<TKey, TValue>(name, locators, serverGroup, redundancy, subscription, 5, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(300));
    }

    public static Pool/*<TKey, TValue>*/ CreatePool<TKey, TValue>(string name, string locators, string serverGroup,
      int redundancy, bool subscription, bool prSingleHop, bool threadLocal = false)
    {
      return CreatePool<TKey, TValue>(name, locators, serverGroup, redundancy, subscription, -1, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(300), false, prSingleHop, threadLocal);
    }

    public static Pool/*<TKey, TValue>*/ CreatePool<TKey, TValue>(string name, string locators, string serverGroup,
      int redundancy, bool subscription, int numConnections, bool isMultiuserMode)
    {
      return CreatePool<TKey, TValue>(name, locators, serverGroup, redundancy, subscription, numConnections, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(300), isMultiuserMode);
    }

    public static Pool/*<TKey, TValue>*/ CreatePool<TKey, TValue>(string name, string locators, string serverGroup,
      int redundancy, bool subscription, int numConnections)
    {
      return CreatePool<TKey, TValue>(name, locators, serverGroup, redundancy, subscription, numConnections, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(300));
    }

    public static Pool/*<TKey, TValue>*/ CreatePool<TKey, TValue>(string name, string locators, string serverGroup,
      int redundancy, bool subscription, TimeSpan ackInterval, TimeSpan dupCheckLife)
    {
      return CreatePool<TKey, TValue>(name, locators, serverGroup, redundancy, subscription,
        5, ackInterval, dupCheckLife);
    }

    public static Pool/*<TKey, TValue>*/ CreatePool<TKey, TValue>(string name, string locators, string serverGroup,
      int redundancy, bool subscription, int numConnections, TimeSpan ackInterval, TimeSpan dupCheckLife)
    {
      return CreatePool<TKey, TValue>(name, locators, serverGroup, redundancy, subscription, numConnections, ackInterval, dupCheckLife, false);
    }

    public static Pool/*<TKey, TValue>*/ CreatePool<TKey, TValue>(string name, string locators, string serverGroup,
      int redundancy, bool subscription, int numConnections, TimeSpan ackInterval, TimeSpan dupCheckLife, bool isMultiuserMode, bool prSingleHop = true, bool threadLocal = false)
    {
      Init();

      Pool/*<TKey, TValue>*/ existing = m_cache.GetPoolManager().Find(name);

      if (existing == null)
      {
        PoolFactory/*<TKey, TValue>*/ fact = m_cache.GetPoolFactory();
        if (locators != null)
        {
          string[] list = locators.Split(',');
          foreach (string item in list)
          {
            string[] parts = item.Split(':');
            fact.AddLocator(parts[0], int.Parse(parts[1]));
          }
        }
        else
        {
          Util.Log("No locators or servers specified for pool");
        }
        if (serverGroup != null)
        {
          fact.SetServerGroup(serverGroup);
        }
        fact.SetSubscriptionRedundancy(redundancy);
        fact.SetSubscriptionEnabled(subscription);
        fact.SetSubscriptionAckInterval(ackInterval);
        fact.SetSubscriptionMessageTrackingTimeout(dupCheckLife);
        fact.SetMultiuserAuthentication(isMultiuserMode);
        fact.SetPRSingleHopEnabled(prSingleHop);
        fact.SetThreadLocalConnections(threadLocal);
        Util.Log("SingleHop set to {0}", prSingleHop);
        Util.Log("ThreadLocal = {0} ", threadLocal);
        Util.Log("numConnections set to {0}", numConnections);
        if (numConnections >= 0)
        {
          fact.SetMinConnections(numConnections);
          fact.SetMaxConnections(numConnections);
        }
        Pool/*<TKey, TValue>*/ pool = fact.Create(name);
        if (pool == null)
        {
          Util.Log("Pool creation failed");
        }
        return pool;
      }
      else
      {
        return existing;
      }
    }

    public static IRegion<TKey, TValue> CreateTCRegion_Pool<TKey, TValue>(string name, bool ack, bool caching,
      ICacheListener<TKey, TValue> listener, string locators, string poolName, bool clientNotification)
    {
      return CreateTCRegion_Pool(name, ack, caching, listener, locators, poolName,
        clientNotification, false, false);
    }

    public static void CreateTCRegion_Pool_AD1(string name, bool ack, bool caching,
       string locators, string poolName, bool clientNotification, bool cloningEnable)
    {
      CreateTCRegion_Pool_AD<object, object>(name, ack, caching, null, locators, poolName, clientNotification, false, cloningEnable);
    }

    public static IRegion<TKey, TValue> CreateTCRegion_Pool_AD<TKey, TValue>(string name, bool ack, bool caching,
      ICacheListener<TKey, TValue> listener, string locators, string poolName, bool clientNotification, bool ssl,
      bool cloningEnabled)
    {
      Properties<string, string> sysProps = new Properties<string, string>();
      if (ssl)
      {
        string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
        sysProps.Insert("ssl-enabled", "true");
        sysProps.Insert("ssl-keystore", keystore + "/client_keystore.pem");
        sysProps.Insert("ssl-truststore", keystore + "/client_truststore.pem");
      }
      InitConfig(sysProps);

      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      if (m_cache.GetPoolManager().Find(poolName) == null)
      {
        PoolFactory/*<TKey, TValue>*/ fact = m_cache.GetPoolFactory();
        fact.SetSubscriptionEnabled(clientNotification);
        if (locators != null)
        {
          string[] list = locators.Split(',');
          foreach (string item in list)
          {
            string[] parts = item.Split(':');
            fact.AddLocator(parts[0], int.Parse(parts[1]));
          }
        }
        else
        {
          Util.Log("No locators or servers specified for pool");
        }
        Pool/*<TKey, TValue>*/ pool = fact.Create(poolName);
        if (pool == null)
        {
          Util.Log("Pool creation failed");
        }
      }

      RegionFactory regionFactory = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetInitialCapacity(100000).SetPoolName(poolName).SetCloningEnabled(cloningEnabled)
        .SetCachingEnabled(caching);

      if (listener != null)
      {
        regionFactory.SetCacheListener(listener);
      }
      else
      {
        Util.Log("Listener is null {0}", listener);
      }

      region = regionFactory.Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      Util.Log("IRegion<object, object> {0} has been created with attributes:{1}",
        name, RegionAttributesToString<TKey, TValue>(region.Attributes));
      return region;
    }

    public static void CreateTCRegion_Pool_MDS(string name, bool ack, bool caching,
        string locators, string poolName, bool clientNotification, bool ssl,
       bool cloningEnabled)
    {
      CacheHelper.CreateTCRegion_Pool<object, object>(name, true, caching,
         null, locators, poolName, clientNotification, ssl, false);
    }

    public static IRegion<TKey, TValue> CreateTCRegion_Pool<TKey, TValue>(string name, bool ack, bool caching,
      ICacheListener<TKey, TValue> listener, string locators, string poolName, bool clientNotification, bool ssl,
      bool cloningEnabled)
    {
      if (ssl)
      {
        Properties<string, string> sysProps = new Properties<string, string>();
        string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
        sysProps.Insert("ssl-enabled", "true");
        sysProps.Insert("ssl-keystore", keystore + "/client_keystore.pem");
        sysProps.Insert("ssl-truststore", keystore + "/client_truststore.pem");
        InitConfig(sysProps);
      }
      else
      {
        Init();
      }
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }
      Pool pl = m_cache.GetPoolManager().Find(poolName);
      if (pl != null)
      {
        Util.Log("Pool is not closed " + poolName);
      }
      if (pl == null)
      {
        PoolFactory/*<TKey, TValue>*/ fact = m_cache.GetPoolFactory();
        fact.SetSubscriptionEnabled(clientNotification);
        if (locators != null)
        {
          string[] list = locators.Split(',');
          foreach (string item in list)
          {
            string[] parts = item.Split(':');
            fact.AddLocator(parts[0], int.Parse(parts[1]));
          }
        }
        else
        {
          Util.Log("No locators or servers specified for pool");
        }
        Pool/*<TKey, TValue>*/ pool = fact.Create(poolName);
        if (pool == null)
        {
          Util.Log("Pool creation failed");
        }
      }
      Util.Log(" caching enable " + caching);
      RegionFactory regionFactory = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetInitialCapacity(100000).SetPoolName(poolName).SetCloningEnabled(cloningEnabled)
        .SetCachingEnabled(caching);

      if (listener != null)
      {
        regionFactory.SetCacheListener(listener);
      }
      else
      {
        Util.Log("Listener is null {0}", listener);
      }

      region = regionFactory.SetPoolName(poolName).Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      Util.Log("IRegion<object, object> {0} has been created with attributes:{1}",
        name, RegionAttributesToString(region.Attributes));
      return region;
    }

    public static IRegion<TKey, TValue> CreateTCRegion_Pool2<TKey, TValue>(string name, bool ack, bool caching,
      ICacheListener<TKey, TValue> listener, string locators, string poolName, bool clientNotification, bool ssl,
      bool cloningEnabled, bool pr)
    {
      if (ssl)
      {
        Properties<string, string> sysProps = new Properties<string, string>();
        string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
        sysProps.Insert("ssl-enabled", "true");
        sysProps.Insert("ssl-keystore", keystore + "/client_keystore.pem");
        sysProps.Insert("ssl-truststore", keystore + "/client_truststore.pem");
        InitConfig(sysProps);
      }
      else
      {
        Init();
      }
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      if (m_cache.GetPoolManager().Find(poolName) == null)
      {
        PoolFactory/*<TKey, TValue>*/ fact = m_cache.GetPoolFactory();
        fact.SetSubscriptionEnabled(clientNotification);
        if (locators != null)
        {
          string[] list = locators.Split(',');
          foreach (string item in list)
          {
            string[] parts = item.Split(':');
            fact.AddLocator(parts[0], int.Parse(parts[1]));
          }
        }
        else
        {
          Util.Log("No locators or servers specified for pool");
        }
        Pool/*<TKey, TValue>*/ pool = fact.Create(poolName);
        if (pool == null)
        {
          Util.Log("Pool creation failed");
        }
      }

      RegionFactory regionFactory = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetInitialCapacity(100000).SetPoolName(poolName).SetCloningEnabled(cloningEnabled)
        .SetCachingEnabled(caching);

      if (listener != null)
      {
        regionFactory.SetCacheListener(listener);
      }
      else
      {
        Util.Log("Listener is null {0}", listener);
      }

      if (pr)
      {
        Util.Log("setting custom partition resolver");
        regionFactory.SetPartitionResolver(CustomPartitionResolver<object>.Create());
      }
      else
      {
        Util.Log("Resolver is null {0}", pr);
      }
      region = regionFactory.Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      Util.Log("IRegion<object, object> {0} has been created with attributes:{1}",
        name, RegionAttributesToString(region.Attributes));
      return region;
    }

    public static IRegion<TKey, TValue> CreateLRUTCRegion_Pool<TKey, TValue>(string name, bool ack, bool caching,
     ICacheListener<TKey, TValue> listener, string locators, string poolName, bool clientNotification, uint lru)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      if (m_cache.GetPoolManager().Find(poolName) == null)
      {
        PoolFactory/*<TKey, TValue>*/ fact = m_cache.GetPoolFactory();
        fact.SetSubscriptionEnabled(clientNotification);
        if (locators != null)
        {
          string[] list = locators.Split(',');
          foreach (string item in list)
          {
            string[] parts = item.Split(':');
            fact.AddLocator(parts[0], int.Parse(parts[1]));
          }
        }
        else
        {
          Util.Log("No locators or servers specified for pool");
        }
        Pool/*<TKey, TValue>*/ pool = fact.Create(poolName);
        if (pool == null)
        {
          Util.Log("Pool creation failed");
        }
      }

      var sqLiteProps = Properties<string, string>.Create();
      sqLiteProps.Insert("PageSize", "65536");
      sqLiteProps.Insert("MaxFileSize", "512000000");
      sqLiteProps.Insert("MaxPageCount", "1073741823");

      String sqlite_dir = "SqLiteRegionData" + Process.GetCurrentProcess().Id.ToString();
      sqLiteProps.Insert("PersistenceDirectory", sqlite_dir);

      RegionFactory regionFactory = m_cache
        .CreateRegionFactory(RegionShortcut.CACHING_PROXY_ENTRY_LRU)
        .SetDiskPolicy(DiskPolicyType.Overflows)
        .SetInitialCapacity(100000).SetPoolName(poolName)
        .SetCachingEnabled(caching).SetLruEntriesLimit(lru)
        .SetPersistenceManager("SqLiteImpl", "createSqLiteInstance", sqLiteProps);

      if (listener != null)
      {
        regionFactory.SetCacheListener(listener);
      }
      else
      {
        Util.Log("Listener is null {0}", listener);
      }

      region = regionFactory.Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      Util.Log("IRegion<object, object> {0} has been created with attributes:{1}",
        name, RegionAttributesToString(region.Attributes));
      return region;
    }

    public static IRegion<TKey, TValue> CreateTCRegion_Pool<TKey, TValue>(string name, bool ack, bool caching,
      ICacheListener<TKey, TValue> listener, string locators, string poolName, bool clientNotification, string serverGroup)
    {
      Init();
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      if (m_cache.GetPoolManager().Find(poolName) == null)
      {
        PoolFactory/*<TKey, TValue>*/ fact = m_cache.GetPoolFactory();
        fact.SetSubscriptionEnabled(clientNotification);
        if (serverGroup != null)
        {
          fact.SetServerGroup(serverGroup);
        }
        if (locators != null)
        {
          string[] list = locators.Split(',');
          foreach (string item in list)
          {
            string[] parts = item.Split(':');
            fact.AddLocator(parts[0], int.Parse(parts[1]));
          }
        }
        else
        {
          Util.Log("No locators or servers specified for pool");
        }
        Pool/*<TKey, TValue>*/ pool = fact.Create(poolName);
        if (pool == null)
        {
          Util.Log("Pool creation failed");
        }
      }

      RegionFactory regionFactory = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetInitialCapacity(100000).SetPoolName(poolName)
        .SetCachingEnabled(caching);

      if (listener != null)
      {
        regionFactory.SetCacheListener(listener);
      }
      else
      {
        Util.Log("Listener is null {0}", listener);
      }

      region = regionFactory.SetPoolName(poolName).SetInitialCapacity(100000).SetCachingEnabled(caching).SetCacheListener(listener).Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      Util.Log("IRegion<object, object> {0} has been created with attributes:{1}",
        name, RegionAttributesToString(region.Attributes));
      return region;
    }

    public static IRegion<TKey, TValue> CreateTCRegion_Pool1<TKey, TValue>(string name, bool ack, bool caching,
      ICacheListener<TKey, TValue> listener, string locators, string poolName, bool clientNotification, bool ssl,
      bool cloningEnabled, IPartitionResolver<int, TValue> pr)
    {
      if (ssl)
      {
        Properties<string, string> sysProps = new Properties<string, string>();
        string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
        sysProps.Insert("ssl-enabled", "true");
        sysProps.Insert("ssl-keystore", keystore + "/client_keystore.pem");
        sysProps.Insert("ssl-truststore", keystore + "/client_truststore.pem");
        InitConfig(sysProps);
      }
      else
      {
        Init();
      }
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(name);
      if ((region != null) && !region.IsDestroyed)
      {
        region.GetLocalView().DestroyRegion();
        Assert.IsTrue(region.IsDestroyed, "IRegion<object, object> {0} was not destroyed.", name);
      }

      if (m_cache.GetPoolManager().Find(poolName) == null)
      {
        PoolFactory fact = m_cache.GetPoolFactory();
        fact.SetSubscriptionEnabled(clientNotification);
        if (locators != null)
        {
          string[] list = locators.Split(',');
          foreach (string item in list)
          {
            string[] parts = item.Split(':');
            fact.AddLocator(parts[0], int.Parse(parts[1]));
          }
        }
        else
        {
          Util.Log("No locators or servers specified for pool");
        }
        Pool pool = fact.Create(poolName);
        if (pool == null)
        {
          Util.Log("Pool creation failed");
        }
      }

      RegionFactory regionFactory = m_cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetInitialCapacity(100000).SetPoolName(poolName).SetCloningEnabled(cloningEnabled)
        .SetCachingEnabled(caching);

      if (listener != null)
      {
        regionFactory.SetCacheListener(listener);
      }
      else
      {
        Util.Log("Listener is null {0}", listener);
      }

      if (pr != null)
      {
        Util.Log("setting custom partition resolver {0} ", pr.GetName());
        regionFactory.SetPartitionResolver(pr);
      }
      else
      {
        Util.Log("Resolver is null {0}", pr);
      }
      region = regionFactory.Create<TKey, TValue>(name);

      Assert.IsNotNull(region, "IRegion<object, object> {0} was not created.", name);
      m_currRegion = region as IRegion<object, object>;
      Util.Log("IRegion<object, object> {0} has been created with attributes:{1}",
        name, RegionAttributesToString(region.Attributes));
      return region;
    }

    public static void DestroyRegion<TKey, TValue>(string name, bool local, bool verify)
    {
      IRegion<TKey, TValue> region;
      if (verify)
      {
        region = GetVerifyRegion<TKey, TValue>(name);
      }
      else
      {
        region = GetRegion<TKey, TValue>(name);
      }
      if (region != null)
      {
        if (local)
        {
          region.GetLocalView().DestroyRegion();
          Util.Log("Locally destroyed region {0}", name);
        }
        else
        {
          region.DestroyRegion();
          Util.Log("Destroyed region {0}", name);
        }
      }
    }

    public static void DestroyAllRegions<TKey, TValue>(bool local)
    {
      if (m_cache != null && !m_cache.IsClosed)
      {
        IRegion<TKey, TValue>[] regions = /*(Region<TKey, TValue>)*/m_cache.RootRegions<TKey, TValue>();
        if (regions != null)
        {
          foreach (IRegion<TKey, TValue> region in regions)
          {
            if (local)
            {
              region.GetLocalView().DestroyRegion();
            }
            else
            {
              region.DestroyRegion();
            }
          }
        }
      }
    }

    public static void InvalidateRegionNonGeneric(string name, bool local, bool verify)
    {
      InvalidateRegion<object, object>(name, local, verify);
    }
    public static void InvalidateRegion<TKey, TValue>(string name, bool local, bool verify)
    {
      IRegion<TKey, TValue> region;
      if (verify)
      {
        region = GetVerifyRegion<TKey, TValue>(name);
        Util.Log("InvalidateRegion: GetVerifyRegion done for {0}", name);
      }
      else
      {
        region = GetRegion<TKey, TValue>(name);
        Util.Log("InvalidateRegion: GetRegion done for {0}", name);
      }
      if (region != null)
      {
        if (local)
        {
          Util.Log("Locally invaliding region {0}", name);
          region.GetLocalView().InvalidateRegion();
          Util.Log("Locally invalidated region {0}", name);
        }
        else
        {
          Util.Log("Invalidating region {0}", name);
          region.InvalidateRegion();
          Util.Log("Invalidated region {0}", name);
        }
      }
    }

    #endregion

    #region Functions to obtain a region

    public static IRegion<TKey, TValue> GetRegion<TKey, TValue>(string path)
    {
      if (m_cache != null)
      {
        return m_cache.GetRegion<TKey, TValue>(path);
      }
      return null;
    }

    public static Properties<string, object> GetPkcsCredentialsForMU(Properties<string, string> credentials)
    {
      if (credentials == null)
        return null;
      var target = Properties<string, object>.Create();
      var psto = new PropsStringToObject(target);
      credentials.ForEach(new PropertyVisitorGeneric<string, string>(psto.Visit));
      return target;
    }

    public static IRegion<TKey, TValue> GetRegion<TKey, TValue>(string path, Properties<string, string> credentials)
    {
      if (m_cache != null)
      {
        Util.Log("GetRegion " + m_cacheForMultiUser);
        if (m_cacheForMultiUser == null)
        {
          IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(path);
          Assert.IsNotNull(region, "IRegion<object, object> [" + path + "] not found.");
          Assert.IsNotNull(region.Attributes.PoolName, "IRegion<object, object> is created without pool.");

          Pool/*<TKey, TValue>*/ pool = m_cache.GetPoolManager().Find(region.Attributes.PoolName);

          Assert.IsNotNull(pool, "Pool is null in GetVerifyRegion.");

          //m_cacheForMultiUser = pool.CreateSecureUserCache(credentials);
          m_cacheForMultiUser = m_cache.CreateAuthenticatedView(GetPkcsCredentialsForMU(credentials), pool.Name);

          return m_cacheForMultiUser.GetRegion<TKey, TValue>(path);
        }
        else
          return m_cacheForMultiUser.GetRegion<TKey, TValue>(path);
      }
      return null;
    }

    public static IRegionService getMultiuserCache(Properties<string, string> credentials)
    {
      if (m_cacheForMultiUser == null)
      {
        Pool/*<TKey, TValue>*/ pool = m_cache.GetPoolManager().Find("__TESTPOOL1_");

        Assert.IsNotNull(pool, "Pool is null in getMultiuserCache.");
        Assert.IsTrue(!pool.Destroyed);

        //m_cacheForMultiUser = pool.CreateSecureUserCache(credentials);
        m_cacheForMultiUser = m_cache.CreateAuthenticatedView(GetPkcsCredentialsForMU(credentials), pool.Name);

        return m_cacheForMultiUser;
      }
      else
        return m_cacheForMultiUser;
    }

    public static IRegion<TKey, TValue> GetVerifyRegion<TKey, TValue>(string path)
    {
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(path);

      Assert.IsNotNull(region, "IRegion<object, object> [" + path + "] not found.");
      Util.Log("Found region '{0}'", path);
      return region;
    }

    public static IRegion<TKey, TValue> GetVerifyRegion<TKey, TValue>(string path, Properties<string, string> credentials)
    {
      Util.Log("GetVerifyRegion " + m_cacheForMultiUser);
      if (m_cacheForMultiUser == null)
      {
        IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(path);
        Assert.IsNotNull(region, "IRegion<object, object> [" + path + "] not found.");
        Assert.IsNotNull(region.Attributes.PoolName, "IRegion<object, object> is created without pool.");

        Pool/*<TKey, TValue>*/ pool = m_cache.GetPoolManager().Find(region.Attributes.PoolName);

        Assert.IsNotNull(pool, "Pool is null in GetVerifyRegion.");

        //m_cacheForMultiUser = pool.CreateSecureUserCache(credentials);
        m_cacheForMultiUser = m_cache.CreateAuthenticatedView(GetPkcsCredentialsForMU(credentials), pool.Name);

        return m_cacheForMultiUser.GetRegion<TKey, TValue>(path);
      }
      else
        return m_cacheForMultiUser.GetRegion<TKey, TValue>(path);
    }

    public static void VerifyRegion<TKey, TValue>(string path)
    {
      GetVerifyRegion<TKey, TValue>(path);
    }

    public static void VerifyNoRegion<TKey, TValue>(string path)
    {
      IRegion<TKey, TValue> region = GetRegion<TKey, TValue>(path);
      Assert.IsNull(region, "IRegion<object, object> [" + path + "] should not exist.");
    }

    #endregion

    #region Functions to start/stop a java cacheserver for Thin Client regions

    public static void SetupJavaServers(params string[] cacheXmls)
    {
      SetupJavaServers(false, cacheXmls);
    }

    public static void setPorts(int s1, int s2, int s3, int l1, int l2)
    {
      HOST_PORT_1 = s1;
      HOST_PORT_2 = s1;
      HOST_PORT_3 = s3;
      LOCATOR_PORT_1 = l1;
      LOCATOR_PORT_2 = l2;
    }

    public static void SetupJavaServers(bool locators, params string[] cacheXmls)
    {
      createRandomPorts();
      m_cacheXmls = cacheXmls;
      m_gfeDir = Util.GetEnvironmentVariable("GFE_DIR");
      Assert.IsNotNull(m_gfeDir, "GFE_DIR is not set.");
      Assert.IsNotEmpty(m_gfeDir, "GFE_DIR is not set.");
      gfsh = Path.Combine(m_gfeDir, "bin", "gfsh.bat");
      m_gfeLogLevel = Util.GetEnvironmentVariable("GFE_LOGLEVEL");
      m_gfeSecLogLevel = Util.GetEnvironmentVariable("GFE_SECLOGLEVEL");
      if (m_gfeLogLevel == null || m_gfeLogLevel.Length == 0)
      {
        m_gfeLogLevel = "config";
      }
      if (m_gfeSecLogLevel == null || m_gfeSecLogLevel.Length == 0)
      {
        m_gfeSecLogLevel = "config";
      }

      Match mt = Regex.Match(m_gfeDir, "^[^:]+:[0-9]+(,[^:]+:[0-9]+)*$");
      if (mt != null && mt.Length > 0)
      {
        // The GFE_DIR is for a remote server; contains an end-point list
        m_endpoints = m_gfeDir;
        m_localServer = false;
      }
      else if (cacheXmls != null)
      {
        // Assume the GFE_DIR is for a local server

        for (int i = 0; i < cacheXmls.Length; i++)
        {
          string cacheXml = cacheXmls[i];
          Assert.IsNotNull(cacheXml, "cacheXml is not set for Java cacheserver.");
          Assert.IsNotEmpty(cacheXml, "cacheXml is not set for Java cacheserver.");

          var duplicateFile = Path.Combine(MakeTempDirectory(), cacheXml);
          cacheXml = Path.Combine(Directory.GetCurrentDirectory(), cacheXml);
          createDuplicateXMLFile(cacheXml, duplicateFile);
          cacheXmls[i] = duplicateFile;

          // Find the port number from the given cache.xml
          XmlDocument xmlDoc = new XmlDocument();
          xmlDoc.XmlResolver = null;
          xmlDoc.Load(duplicateFile);

          XmlNamespaceManager ns = new XmlNamespaceManager(xmlDoc.NameTable);
          ns.AddNamespace("geode", "http://geode.apache.org/schema/cache");
          XmlNodeList serverNodeList = xmlDoc.SelectNodes("//geode:cache-server", ns);

          if (m_endpoints == null)
          {
            m_endpoints = System.Net.Dns.GetHostEntry("localhost").HostName + ":" + serverNodeList[0].Attributes["port"].Value;
          }
          else
          {
            m_endpoints += "," + System.Net.Dns.GetHostEntry("localhost").HostName + ":" + serverNodeList[0].Attributes["port"].Value;
          }
        }
        Util.Log("JAVA server endpoints: " + m_endpoints);
      }
    }

    public static void createRandomPorts()
    {
      if (HOST_PORT_1 == 0)
      {
        HOST_PORT_1 = Util.GetAvailablePort();
        HOST_PORT_2 = Util.GetAvailablePort();
        HOST_PORT_3 = Util.GetAvailablePort();
        HOST_PORT_4 = Util.GetAvailablePort();
      }

      if (LOCATOR_PORT_1 == 0)
      {
        LOCATOR_PORT_1 = Util.GetAvailablePort();
        LOCATOR_PORT_2 = Util.GetAvailablePort();
        LOCATOR_PORT_3 = Util.GetAvailablePort();
        LOCATOR_PORT_4 = Util.GetAvailablePort();
      }

      if (JMX_MANAGER_PORT == 0)
      {
        JMX_MANAGER_PORT = Util.GetAvailablePort();
      }

    }

    public static void createDuplicateXMLFile(string orignalFilename, string duplicateFilename)
    {
      string cachexmlstring = File.ReadAllText(orignalFilename);
      cachexmlstring = cachexmlstring.Replace("HOST_PORT1", HOST_PORT_1.ToString());
      cachexmlstring = cachexmlstring.Replace("HOST_PORT2", HOST_PORT_2.ToString());
      cachexmlstring = cachexmlstring.Replace("HOST_PORT3", HOST_PORT_3.ToString());
      cachexmlstring = cachexmlstring.Replace("HOST_PORT4", HOST_PORT_4.ToString());

      cachexmlstring = cachexmlstring.Replace("LOC_PORT1", LOCATOR_PORT_1.ToString());
      cachexmlstring = cachexmlstring.Replace("LOC_PORT2", LOCATOR_PORT_2.ToString());
      cachexmlstring = cachexmlstring.Replace("LOC_PORT3", LOCATOR_PORT_3.ToString());
      cachexmlstring = cachexmlstring.Replace("LOC_PORT4", LOCATOR_PORT_4.ToString());

      File.Create(duplicateFilename).Close();
      File.WriteAllText(duplicateFilename, cachexmlstring);
    }

    public static void StartJavaLocator(int locatorNum, string locatorName)
    {
      StartJavaLocator(locatorNum, locatorName, null);
    }

    public static void StartJavaLocator(int locatorNum, string locatorName,
      string extraLocatorArgs)
    {
      StartJavaLocator(locatorNum, locatorName, extraLocatorArgs, false);
    }

    public static void StartJavaLocator(int locatorNum, string locatorName,
      string extraLocatorArgs, bool ssl)
    {
      if (m_localServer)
      {
        var startDir = MakeTempDirectory();
        Util.Log("Starting locator {0} in directory {1}.", locatorNum, startDir);
        try
        {
          TextWriter tw = new StreamWriter(Path.Combine(startDir, "geode.properties"), false);
          tw.WriteLine("locators=localhost[{0}],localhost[{1}],localhost[{2}]", LOCATOR_PORT_1, LOCATOR_PORT_2, LOCATOR_PORT_3);
          if (ssl)
          {
            tw.WriteLine("ssl-enabled=true");
            tw.WriteLine("ssl-require-authentication=true");
            tw.WriteLine("ssl-ciphers=SSL_RSA_WITH_NULL_MD5");
            tw.WriteLine("mcast-port=0");
          }
          tw.Close();
        }
        catch (Exception ex)
        {
          Assert.Fail("Locator property file creation failed: {0}: {1}", ex.GetType().Name, ex.Message);
        }

        string locatorPort = " --port=" + getLocatorPort(locatorNum);
        if (extraLocatorArgs != null)
        {
          extraLocatorArgs = ' ' + extraLocatorArgs + locatorPort;
        }
        else
        {
          extraLocatorArgs = locatorPort;
        }

        extraLocatorArgs += " --J=-Dgemfire.jmx-manager-port=" + JMX_MANAGER_PORT;

        if (ssl)
        {
          string sslArgs = string.Empty;
          string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
          sslArgs += " --J=-Djavax.net.ssl.keyStore=" + keystore + "/server_keystore.jks ";
          sslArgs += " --J=-Djavax.net.ssl.keyStorePassword=gemstone ";
          sslArgs += " --J=-Djavax.net.ssl.trustStore=" + keystore + "/server_truststore.jks  ";
          sslArgs += " --J=-Djavax.net.ssl.trustStorePassword=gemstone ";
          extraLocatorArgs += sslArgs;
        }

        string locatorArgs = LocatorStartArgs + " --name=" + locatorName + " --dir=" + startDir + extraLocatorArgs + " --http-service-port=0";

        var exitCode = ExecuteGfsh(locatorArgs);
        if (0 != exitCode)
        {
          KillLocatorByPidFile(startDir);
        }
        Assert.AreEqual(0, exitCode, "Failed to start locator.");

        m_runningLocators[locatorNum] = startDir;
        if (m_locators == null)
        {
          m_locators = "localhost:" + getLocatorPort(locatorNum);
        }
        else
        {
          m_locators += ",localhost:" + getLocatorPort(locatorNum);
        }
        Util.Log("JAVA locator endpoints: " + m_locators);
      }
    }

    private static void KillLocatorByPidFile(string startDir)
    {
      KillByPidFile(Path.Combine(startDir, "vf.gf.locator.pid"));
    }

    static int getLocatorPort(int num)
    {
      switch (num)
      {
        case 1:
          return LOCATOR_PORT_1;
        case 2:
          return LOCATOR_PORT_2;
        case 3:
          return LOCATOR_PORT_3;
        default:
          return LOCATOR_PORT_1;
      }
    }

    //this is for start locator independetly(will not see each other)
    public static void StartJavaLocator_MDS(int locatorNum, string locatorName,
      string extraLocatorArgs, int dsId)
    {
      if (m_localServer)
      {
        var startDir = MakeTempDirectory();
        Util.Log("Starting locator {0} in directory {1}.", locatorNum, startDir);
        try
        {
          TextWriter tw = new StreamWriter(Path.Combine(startDir, "geode.properties"), false);
          //tw.WriteLine("locators=localhost[{0}],localhost[{1}],localhost[{2}]", LOCATOR_PORT_1, LOCATOR_PORT_2, LOCATOR_PORT_3);
          tw.WriteLine("distributed-system-id=" + dsId);
          tw.WriteLine("mcast-port=0");
          tw.Close();
        }
        catch (Exception ex)
        {
          Assert.Fail("Locator property file creation failed: {0}: {1}", ex.GetType().Name, ex.Message);
        }

        if (dsId == 1)
          m_locatorFirst = "localhost:" + getLocatorPort(locatorNum);
        else
          m_locatorSecond = "localhost:" + getLocatorPort(locatorNum);

        string locatorPort = " --port=" + getLocatorPort(locatorNum);
        if (extraLocatorArgs != null)
        {
          extraLocatorArgs = ' ' + extraLocatorArgs + locatorPort;
        }
        else
        {
          extraLocatorArgs = locatorPort;
        }
        string locatorArgs = LocatorStartArgs + " --name=" + locatorName + " --dir=" + startDir + extraLocatorArgs + " --http-service-port=0";

        var exitCode = ExecuteGfsh(locatorArgs);
        if (0 != exitCode)
        {
          KillLocatorByPidFile(startDir);
        }
        Assert.AreEqual(0, exitCode, "Failed to start locator MDS.");

        m_runningLocators[locatorNum] = startDir;
        if (m_locators == null)
        {
          m_locators = "localhost[" + getLocatorPort(locatorNum) + "]";
        }
        else
        {
          m_locators += ",localhost[" + getLocatorPort(locatorNum) + "]";
        }
        Util.Log("JAVA locator endpoints: " + m_locators);
      }
    }

    public static void StartJavaServerWithLocators(int serverNum, string serverName, int numLocators)
    {
      StartJavaServerWithLocators(serverNum, serverName, numLocators, false);
    }

    public static void StartJavaServerWithLocators(int serverNum, string serverName, int numLocators, bool ssl)
    {
      string extraServerArgs = "--locators=";
      for (int locator = 0; locator < numLocators; locator++)
      {
        if (locator > 0)
        {
          extraServerArgs += ",";
        }
        extraServerArgs += "localhost[" + getLocatorPort(locator + 1) + "]";
      }
      if (ssl)
      {
        string sslArgs = String.Empty;
        sslArgs += " ssl-enabled=true ssl-require-authentication=true ssl-ciphers=SSL_RSA_WITH_NULL_MD5 ";
        string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
        sslArgs += " -J=-Djavax.net.ssl.keyStore=" + keystore + "/server_keystore.jks ";
        sslArgs += " -J=-Djavax.net.ssl.keyStorePassword=gemstone ";
        sslArgs += " -J=-Djavax.net.ssl.trustStore=" + keystore + "/server_truststore.jks  ";
        sslArgs += " -J=-Djavax.net.ssl.trustStorePassword=gemstone ";
        extraServerArgs += sslArgs;
      }
      StartJavaServer(serverNum, serverName, extraServerArgs);
    }

    //this is to start multiple DS
    public static void StartJavaServerWithLocator_MDS(int serverNum, string serverName, int locatorNumber)
    {
      string extraServerArgs = "--locators=";
      extraServerArgs += "localhost[" + getLocatorPort(locatorNumber) + "]";

      StartJavaServer(serverNum, serverName, extraServerArgs);
    }


    public static void StartJavaServer(int serverNum, string serverName)
    {
      StartJavaServer(serverNum, serverName, null);
    }

    public static void StartJavaServerWithLocators(int serverNum, string serverName,
      int numLocators, string extraServerArgs)
    {
      StartJavaServerWithLocators(serverNum, serverName, numLocators, extraServerArgs, false);
    }

    public static void StartJavaServerWithLocators(int serverNum, string serverName,
      int numLocators, string extraServerArgs, bool ssl)
    {
      extraServerArgs += " --locators=";
      for (int locator = 0; locator < numLocators; locator++)
      {
        if (locator > 0)
        {
          extraServerArgs += ",";
        }
        extraServerArgs += "localhost[" + getLocatorPort(locator + 1) + "]";
      }
      if (ssl)
      {
        string sslArgs = String.Empty;
        sslArgs += " ssl-enabled=true ssl-require-authentication=true ssl-ciphers=SSL_RSA_WITH_NULL_MD5 ";
        string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
        sslArgs += " --J=-Djavax.net.ssl.keyStore=" + keystore + "/server_keystore.jks ";
        sslArgs += " --J=-Djavax.net.ssl.keyStorePassword=gemstone ";
        sslArgs += " --J=-Djavax.net.ssl.trustStore=" + keystore + "/server_truststore.jks  ";
        sslArgs += " --J=-Djavax.net.ssl.trustStorePassword=gemstone ";
        extraServerArgs += sslArgs;
      }
      StartJavaServer(serverNum, serverName, extraServerArgs);
    }

    public static void StartJavaServer(int serverNum, string serverName, string extraServerArgs)
    {
      if (m_localServer)
      {
        if (m_cacheXmls == null || serverNum > m_cacheXmls.Length)
        {
          Assert.Fail("SetupJavaServers called with incorrect parameters: " +
            "could not find cache.xml for server number {0}", serverNum);
        }
        string cacheXml = m_cacheXmls[serverNum - 1];
        var startDir = Path.Combine(MakeTempDirectory());
        int port = 0;
        switch (serverNum)
        {
          case 1:
            port = HOST_PORT_1;
            break;
          case 2:
            port = HOST_PORT_2;
            break;
          case 3:
            port = HOST_PORT_3;
            break;
          case 4:
            port = HOST_PORT_4;
            break;
          default:
            throw new InvalidOperationException();
        }
        Util.Log("Starting server {0} in directory {1}.", serverNum, startDir);
        if (startDir != null)
        {
          if (!Directory.Exists(startDir))
          {
            Directory.CreateDirectory(startDir);
          }
        }
        else
        {
          startDir = string.Empty;
        }
        if (extraServerArgs != null)
        {
          extraServerArgs = ' ' + extraServerArgs;
        }

        string classpath = Util.GetEnvironmentVariable("GF_CLASSPATH");

        string serverArgs = JavaServerStartArgs + cacheXml + " --name=" + serverName +
          " --server-port=" + port + " --classpath=" + classpath +
          " --log-level=" + m_gfeLogLevel + " --dir=" + startDir +
          " --J=-Dsecurity-log-level=" + m_gfeSecLogLevel + extraServerArgs;

        var exitCode = ExecuteGfsh(serverArgs);
        if (0 != exitCode)
        {
          KillServerByPidFile(startDir);
        }
        Assert.AreEqual(0, exitCode, "Failed to start server.");

        m_runningJavaServers[serverNum] = startDir;
      }
    }

    private static void KillServerByPidFile(string startDir)
    {
      KillByPidFile(Path.Combine(startDir, "vf.gf.server.pid"));
    }

    static string MakeTempDirectory()
    {
      var tempDirectory = Path.Combine(tempDirectoryRoot, Path.GetRandomFileName());
      Directory.CreateDirectory(tempDirectory);
      return tempDirectory;
    }

    static int ExecuteGfsh(string command)
    {
      Util.Log("ExecuteGfsh: {0}", command);

      using (var process = new Process())
      {
        process.StartInfo.FileName = gfsh;
        process.StartInfo.Arguments = command;
        process.StartInfo.UseShellExecute = false;
        process.StartInfo.RedirectStandardOutput = true;
        process.StartInfo.RedirectStandardError = true;
        process.StartInfo.CreateNoWindow = true;
        process.StartInfo.EnvironmentVariables["JAVA_ARGS"] = "-Xmx256m";

        process.OutputDataReceived += (sender, e) =>
        {
          if (!string.IsNullOrEmpty(e.Data))
          {
            Util.Log("Execute Gfsh stdout: {0}", e.Data);
          }
        };
        process.ErrorDataReceived += (sender, e) =>
        {
          if (!string.IsNullOrEmpty(e.Data))
          {
            Util.Log("Execute Gfsh stderr: {0}", e.Data);
          }
        };

        process.Start();

        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        Util.Log("ExecuteGfsh: Waiting for exit {0}", process.Id);
        if (!process.WaitForExit(MaxWaitMillis))
        {
          Util.Log("ExecuteGfsh: Timeout, killing {0}", process.Id);

          CloseAndIgnore(process.StandardOutput);
          CloseAndIgnore(process.StandardError);

          try
          {
            process.Kill();
          }
          catch (Exception)
          {
            //ignore
          }
        }

        Util.Log("ExecuteGfsh: Exited {0}", process.Id);
        return process.ExitCode;
      }
    }

    public static void CloseAndIgnore(StreamReader streamRead)
    {
      try
      {
        streamRead.Close();
      } 
      catch (Exception)
      {
        // ignored
      }
    }

    public static void StopJavaLocator(int locatorNum)
    {
      StopJavaLocator(locatorNum, true, false);
    }

    public static void StopJavaLocator(int locatorNum, bool verifyLocator)
    {
      StopJavaLocator(locatorNum, verifyLocator, false);
    }

    public static void StopJavaLocator(int locatorNum, bool verifyLocator, bool ssl)
    {
      if (m_localServer)
      {
        // Assume the GFE_DIR is for a local server
        string startDir;
        if (m_runningLocators.TryGetValue(locatorNum, out startDir))
        {
          Util.Log("Stopping locator {0} in directory {1}.", locatorNum, startDir);
          string sslArgs = String.Empty;
          if (ssl)
          {
            string keystore = Util.GetEnvironmentVariable("CPP_TESTOUT") + "/keystore";
            sslArgs += " --J=-Djavax.net.ssl.keyStore=" + keystore + "/server_keystore.jks ";
            sslArgs += " --J=-Djavax.net.ssl.keyStorePassword=gemstone ";
            sslArgs += " --J=-Djavax.net.ssl.trustStore=" + keystore + "/server_truststore.jks  ";
            sslArgs += " --J=-Djavax.net.ssl.trustStorePassword=gemstone ";
            File.Copy(startDir + "/geode.properties", Directory.GetCurrentDirectory() + "/geode.properties", true);
          }

          var exitCode = ExecuteGfsh(LocatorStopArgs + " --dir=" + startDir + sslArgs);
          if (0 != exitCode)
          {
            KillLocatorByPidFile(startDir);
          }

          if (ssl)
          {
            File.Delete(Directory.GetCurrentDirectory() + "/geode.properties");
          }
          m_runningLocators.Remove(locatorNum);
          Util.Log("Locator {0} in directory {1} stopped.", locatorNum, startDir);
        }
        else
        {
          if (verifyLocator)
          {
            Assert.Fail("StopJavaLocator() invoked for a non-existing locator {0}",
              locatorNum);
          }
        }
      }
    }

    public static void StopJavaServer(int serverNum)
    {
      StopJavaServer(serverNum, true);
    }

    public static void StopJavaServer(int serverNum, bool verifyServer)
    {
      if (m_localServer)
      {
        // Assume the GFE_DIR is for a local server
        string startDir;
        if (m_runningJavaServers.TryGetValue(serverNum, out startDir))
        {
          Util.Log("Stopping server {0} in directory {1}.", serverNum, startDir);

          var exitCode = ExecuteGfsh(JavaServerStopArgs + " --dir=" + startDir);
          if (0 != exitCode)
          {
            KillServerByPidFile(startDir);
          }

          m_runningJavaServers.Remove(serverNum);
          Util.Log("Server {0} in directory {1} stopped.", serverNum, startDir);
        }
        else
        {
          if (verifyServer)
          {
            Assert.Fail("StopJavaServer() invoked for a non-existing server {0}",
              serverNum);
          }
        }
      }
    }

    private static void KillByPidFile(string pidFile)
    {
      if (File.Exists(pidFile))
      {
        Util.Log(Util.LogLevel.Info, "PID file {0} found.", pidFile);
        var pid = int.Parse(File.ReadAllText(pidFile));
        try 
        {
          using (var process = Process.GetProcessById(pid))
          {
            Util.Log(Util.LogLevel.Warning, "Killing process {0}.", pid);
            process.Kill();
            if (!process.WaitForExit(MaxWaitMillis))
            {
              Util.Log(Util.LogLevel.Error, "Failed to kill {0}.", pid);
            }
          }
        }
        catch (ArgumentException)
        {
          Util.Log(Util.LogLevel.Info, "Process {0} does not exist.", pid);
        }
      }
    }

    public static void StopJavaServers()
    {
      int[] runningServers = new int[m_runningJavaServers.Count];
      m_runningJavaServers.Keys.CopyTo(runningServers, 0);
      foreach (int serverNum in runningServers)
      {
        StopJavaServer(serverNum);
        Util.Log("Cacheserver {0} stopped.", serverNum);
      }
      m_runningJavaServers.Clear();
    }

    public static void StopJavaLocators()
    {
      int[] runningServers = new int[m_runningLocators.Count];
      m_runningLocators.Keys.CopyTo(runningServers, 0);
      foreach (int serverNum in runningServers)
      {
        StopJavaLocator(serverNum);
        Util.Log("Locator {0} stopped.", serverNum);
      }
      m_runningLocators.Clear();
    }

    public static void ClearEndpoints()
    {
      m_endpoints = null;
    }

    public static void ClearLocators()
    {
      m_locators = null;
    }



    public static void KillJavaProcesses()
    {
      String myQuery = "select * from win32_process where Name ='java.exe' or Name = 'java.exe *32'";
      ObjectQuery objQuery = new ObjectQuery(myQuery);
      ManagementObjectSearcher objSearcher = new ManagementObjectSearcher(objQuery);
      ManagementObjectCollection processList = objSearcher.Get();

      foreach (ManagementObject item in processList)
      {
        try
        {
          int processId = Convert.ToInt32(item["ProcessId"].ToString());
          string commandline = item["CommandLine"].ToString();

          Util.Log("processId:{0} name:{1}", item["ProcessId"], item["Name"]);
          if (commandline.Contains("gemfire.jar"))
          {
            Util.Log("Killing geode process with id {0}", processId);
            Process proc = Process.GetProcessById(processId);
            try
            {
              proc.Kill();
            }
            catch
            {
              //ignore
            }
            proc.WaitForExit(MaxWaitMillis);
          }
          else
          {
            Util.Log("Process with id {0} is not geode process", processId);
          }
        }
        catch (Exception e)
        {
          Console.WriteLine("Error: " + e);
        }
      }

    }

    public static void EndTest()
    {
      Util.Log("Cache Helper EndTest.");
      StopJavaServers();
      StopJavaLocators();
      ClearEndpoints();
      ClearLocators();
      //KillJavaProcesses();
      Util.Log("Cache Helper EndTest completed.");
    }

    #endregion

    #region Utility functions

    public static void ShowKeys(ICollection<Object> cKeys)
    {
      if (cKeys != null)
      {
        for (int i = 0; i < cKeys.Count; i++)
        {
          //TODO ATTACH List TO THE COLLECTION AND UNCOMMENT BELOW LINE
          //Util.Log("Key [{0}] = {1}", i, cKeys[i]);
        }
      }
    }

    public static void ShowValues(ICollection<Object> cValues)
    {
      if (cValues != null)
      {
        for (int i = 0; i < cValues.Count; i++)
        {
          //TODO ATTACH List TO THE COLLECTION AND UNCOMMENT BELOW LINE
          //Util.Log("Value [{0}] = {1}", i, cValues[i]);
        }
      }
    }

    public static string RegionAttributesToString<TKey, TVal>(Apache.Geode.Client.RegionAttributes<TKey, TVal> attrs)
    {
      string poolName = "RegionWithoutPool";

      if (attrs.PoolName != null)
        poolName = attrs.PoolName;

      StringBuilder attrsSB = new StringBuilder();
      attrsSB.Append(Environment.NewLine + "caching: " +
        attrs.CachingEnabled);
      attrsSB.Append(Environment.NewLine + "endpoints: " +
        attrs.Endpoints);
      attrsSB.Append(Environment.NewLine + "clientNotification: " +
        attrs.ClientNotificationEnabled);
      attrsSB.Append(Environment.NewLine + "initialCapacity: " +
        attrs.InitialCapacity);
      attrsSB.Append(Environment.NewLine + "loadFactor: " +
        attrs.LoadFactor);
      attrsSB.Append(Environment.NewLine + "concurrencyLevel: " +
        attrs.ConcurrencyLevel);
      attrsSB.Append(Environment.NewLine + "lruEntriesLimit: " +
        attrs.LruEntriesLimit);
      attrsSB.Append(Environment.NewLine + "lruEvictionAction: " +
        attrs.LruEvictionAction);
      attrsSB.Append(Environment.NewLine + "entryTimeToLive: " +
        attrs.EntryTimeToLive);
      attrsSB.Append(Environment.NewLine + "entryTimeToLiveAction: " +
        attrs.EntryTimeToLiveAction);
      attrsSB.Append(Environment.NewLine + "entryIdleTimeout: " +
        attrs.EntryIdleTimeout);
      attrsSB.Append(Environment.NewLine + "entryIdleTimeoutAction: " +
        attrs.EntryIdleTimeoutAction);
      attrsSB.Append(Environment.NewLine + "regionTimeToLive: " +
        attrs.RegionTimeToLive);
      attrsSB.Append(Environment.NewLine + "regionTimeToLiveAction: " +
        attrs.RegionTimeToLiveAction);
      attrsSB.Append(Environment.NewLine + "regionIdleTimeout: " +
        attrs.RegionIdleTimeout);
      attrsSB.Append(Environment.NewLine + "regionIdleTimeoutAction: " +
        attrs.RegionIdleTimeoutAction);
      attrsSB.Append(Environment.NewLine + "PoolName: " +
        poolName);
      attrsSB.Append(Environment.NewLine);
      return attrsSB.ToString();
    }

    #endregion
  }
}
