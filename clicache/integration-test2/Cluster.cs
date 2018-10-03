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

namespace Apache.Geode.Client.IntegrationTests
{
    public class Cluster : IDisposable
    {
        private int locatorCount_;
        private int serverCount_;
        private bool started_;
        private List<Locator> locators_;
        private List<Server> servers_;
        private string name_;

        public Gfsh Gfsh { get; private set; }

        public Cluster(string name, int locatorCount, int serverCount)
        {
            started_ = false;
            this.Gfsh = new GfshExecute();
            name_ = name;
            locatorCount_ = locatorCount;
            serverCount_ = serverCount;
            locators_ = new List<Locator>();
            servers_ = new List<Server>();
        }

        private bool StartLocators()
        {
            bool success = true;

            for (int i = 0; i < locatorCount_; i++)
            {
                Locator locator = new Locator(this, new List<Locator>(), 
                    name_ + "/locator/" + i.ToString());
                locators_.Add(locator);
                int localResult = locator.Start();
                if (localResult != 0)
                {
                    success = false;
                }
            }
            return success;
        }

        private bool StartServers()
        {
            bool success = true;

            for (int i = 0; i < serverCount_; i++)
            {
                Server server = new Server(this, locators_, 
                    name_ + "/server/" + i.ToString());
                servers_.Add(server);
                int localResult = server.Start();
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
            bool success = false;
            if (!started_)
            {
                RemoveClusterDirectory();
                success = StartLocators();
                if (!StartServers())
                {
                    success = false;
                }
                started_ = true;
            }
            return success;
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
            Address address = new Address();
            address.address = "localhost";
            address.port = cluster.Gfsh.LocatorPort;
            Address = address;
        }

        public Address Address { get; private set; }
        
        public int Start()
        {
            int result = -1;
            if (!started_)
            {
                result = cluster_.Gfsh
                    .start()
                    .locator()
                    .withDir(name_)
                    .withName(name_.Replace('/', '_'))
                    .withBindAddress(Address.address)
                    .withPort(Address.port)
                    .withMaxHeap("256m")
                    .withJmxManagerPort(cluster_.Gfsh.JmxManagerPort)
                    .withHttpServicePort(0)
                    .execute();
                started_ = true;
            }
            return result;
        }

        public int Stop()
        {
            int result = cluster_.Gfsh
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
            Address address = new Address();
            address.address = "localhost";
            address.port = 0;
            Address = address;
        }

        public Address Address { get; private set; }

        public int Start()
        {
            int result = -1;
            if (!started_)
            {
                result = cluster_.Gfsh
                    .start()
                    .server()
                    .withDir(name_)
                    .withName(name_.Replace('/', '_'))
                    .withBindAddress(Address.address)
                    .withPort(Address.port)
                    .withMaxHeap("1g")
                    .execute();
                started_ = true;
            }
            return result;
        }

        public int Stop()
        {
            int result = cluster_.Gfsh
                .stop()
                .server()
                .withDir(name_)
                .execute();
            started_ = false;
            return result;
        }
    }
}
