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
using System.Net;
using System.IO;
using System.Net.Sockets;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    public class Cluster : IDisposable
    {
        private int locatorCount_;
        private int serverCount_;
        private bool started_;
        private List<Locator> locators_;
        private string name_;
        internal int jmxManagerPort = Framework.FreeTcpPort();
        internal string keyStore_ = Environment.CurrentDirectory + "/ServerSslKeys/server_keystore.jks";
        internal string keyStorePassword_ = "gemstone";
        internal string trustStore_ = Environment.CurrentDirectory + "/ServerSslKeys/server_truststore.jks";
        internal string trustStorePassword_ = "gemstone";

        public Gfsh Gfsh { get; private set; }

        public bool UseSSL { get; set; }

        internal PoolFactory ApplyLocators(PoolFactory poolFactory)
        {
            foreach (var locator in locators_)
            {
                poolFactory.AddLocator(locator.Address.address, locator.Address.port);
            }

            return poolFactory;
        }

        public Cluster(ITestOutputHelper output, string name, int locatorCount, int serverCount)
        {
            started_ = false;
            Gfsh = new GfshExecute(output);
            UseSSL = false;
            name_ = name;
            locatorCount_ = locatorCount;
            serverCount_ = serverCount;
            locators_ = new List<Locator>();
            servers_ = new List<Server>();
        }

        private bool StartLocators()
        {
            var success = true;

            for (var i = 0; i < locatorCount_; i++)
            {
                var locator = new Locator(this, new List<Locator>(),
                    name_ + "/locator/" + i.ToString());
                locators_.Add(locator);
                success = (locator.Start() == 0);
            }
            return success;
        }

        private bool StartServers()
        {
            var success = true;

            for (var i = 0; i < serverCount_; i++)
            {
                var server = new Server(this, locators_,
                    name_ + "/server/" + i.ToString());
                servers_.Add(server);
                var localResult = server.Start();
                if (localResult != 0)
                {
                    success = false;
                }
            }
            return success;
        }

        private void RemoveClusterDirectory()
        {
            if (Directory.Exists(name_))
            {
                Directory.Delete(name_, true);
            }
        }

        public bool Start()
        {
            if (!started_)
            {
                RemoveClusterDirectory();
                var locatorSuccess = StartLocators();
                var serverSuccess = StartServers();
                started_ = (locatorSuccess && serverSuccess);
            }
            return (started_);
        }

        public void Dispose()
        {
            if (started_)
            {
                this.Gfsh
                    .shutdown()
                    .withIncludeLocators(true)
                    .execute();
            }
        }

        public Cache CreateCache(IDictionary<string, string> properties)
        {
            var cacheFactory = new CacheFactory();

            cacheFactory
                .Set("log-level", "none")
                .Set("statistic-sampling-enabled", "false");

            foreach (var pair in properties)
            {
                cacheFactory.Set(pair.Key, pair.Value);
            }

            var cache = cacheFactory.Create();

            ApplyLocators(cache.GetPoolFactory()).Create("default");

            return cache;
        }

        public Cache CreateCache()
        {
            return CreateCache(new Dictionary<string, string>());
        }

    }

    public struct Address
    {
        public string address;
        public int port;
    }

    public class Locator
    {
        private Cluster cluster_;
        private string name_;
        private List<Locator> locators_;
        private bool started_;

        public Locator(Cluster cluster, List<Locator> locators, string name)
        {
            cluster_ = cluster;
            locators_ = locators;
            name_ = name;
            var address = new Address();
            address.address = "localhost";
            address.port = Framework.FreeTcpPort();
            Address = address;
        }

        public Address Address { get; private set; }

        public int Start()
        {
            var result = -1;
            if (!started_)
            {
                var locator = cluster_.Gfsh
                    .start()
                    .locator()
                    .withDir(name_)
                    .withName(name_.Replace('/', '_'))
                    .withBindAddress(Address.address)
                    .withPort(Address.port)
                    .withMaxHeap("256m")
                    .withJmxManagerPort(cluster_.jmxManagerPort)
                    .withJmxManagerStart(true)
                    .withHttpServicePort(0);
                if (cluster_.UseSSL)
                {
                   locator
                        .withConnect(false)
                        .withSslEnableComponents("all")
                        .withSslKeyStore(cluster_.keyStore_)
                        .withSslKeyStorePassword(cluster_.keyStorePassword_)
                        .withSslTrustStore(cluster_.trustStore_)
                        .withSslTrustStorePassword(cluster_.trustStorePassword_);
                }
                result = locator.execute();

                if (cluster_.UseSSL)
                {
                    cluster_.Gfsh.connect()
                        .withJmxManager(Address.address, cluster_.jmxManagerPort)
                        .withUseSsl(true)
                        .withKeyStore(cluster_.keyStore_)
                        .withKeyStorePassword(cluster_.keyStorePassword_)
                        .withTrustStore(cluster_.trustStore_)
                        .withTrustStorePassword(cluster_.trustStorePassword_)
                        .execute();
                }

                started_ = true;

            }
            return result;
        }

        public int Stop()
        {
            var result = cluster_.Gfsh
                .stop()
                .locator()
                .withDir(name_)
                .execute();
            started_ = false;
            return result;
        }
    }

    public class Server
    {
        private Cluster cluster_;
        private string name_;
        private List<Locator> locators_;
        private bool started_;

        public Server(Cluster cluster, List<Locator> locators, string name)
        {
            cluster_ = cluster;
            locators_ = locators;
            name_ = name;
            var address = new Address();
            address.address = "localhost";
            address.port = 0;
            Address = address;
        }

        public Address Address { get; private set; }

        public int Start()
        {
            var result = -1;
            if (!started_)
            {
                var server = cluster_.Gfsh
                    .start()
                    .server()
                    .withDir(name_)
                    .withName(name_.Replace('/', '_'))
                    .withBindAddress(Address.address)
                    .withPort(Address.port)
                    .withMaxHeap("1g");
                if (cluster_.UseSSL)
                {
                    server
                        .withSslEnableComponents("all")
                        .withSslKeyStore(cluster_.keyStore_)
                        .withSslKeyStorePassword(cluster_.keyStorePassword_)
                        .withSslTrustStore(cluster_.trustStore_)
                        .withSslTrustStorePassword(cluster_.trustStorePassword_);

                }
                result = server.execute();
                started_ = true;
            }
            return result;
        }

        public int Stop()
        {
            var result = cluster_.Gfsh
                .stop()
                .server()
                .withDir(name_)
                .execute();
            started_ = false;
            return result;
        }
    }
}
