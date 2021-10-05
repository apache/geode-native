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
using System.Diagnostics;
using System.Net;
using System.IO;
using System.Net.Sockets;
using System.Reflection;
using Xunit;
using System.Collections.Generic;

namespace Apache.Geode.Client.IntegrationTests
{
    public abstract class Gfsh 
    {

        //TODO: Understand what C++ Command class is doing.  Why is it a template,
        //when the only <T> class we're passing is void?  How can you call a ctor
        //or a (non-existent?) 'parse' function on void?  So many questions...
        public class Command
        {
            public Command(Gfsh gfsh, string command)
            {
                gfsh_ = gfsh;
                command_ = command;
            }

            public int execute()
            {
                return gfsh_.execute(command_);
            }

            public override string ToString()
            {
                return command_;
            }

            protected Gfsh gfsh_;
            protected string command_;
        }

        public class Start
        {
            public Start(Gfsh gfsh)
            {
                gfsh_ = gfsh;
            }

            public class Server : Command
            {
                public Server(Gfsh gfsh) : base(gfsh, "start server")
                {
                    gfsh_ = gfsh;
                }

                public Server withName(string name)
                {
                    command_ += " --name=" + name;
                    return this;
                }

                public Server withDir(string dir)
                {
                    command_ += " --dir=" + dir;
                    return this;
                }

                public Server withBindAddress(string bindAddress)
                {
                    command_ += " --bind-address=" + bindAddress;
                    return this;
                }

                public Server withPort(int port)
                {
                    command_ += " --server-port=" + port.ToString();
                    return this;
                }

                public Server withLocators(string locators)
                {
                    command_ += " --locators=" + locators;
                    return this;
                }

                public Server withLogLevel(string logLevel)
                {
                    command_ += " --log-level=" + logLevel;
                    return this;
                }

                public Server withMaxHeap(string maxHeap)
                {
                    command_ += " --max-heap=" + maxHeap;
                    return this;
                }

               public Server withSslKeyStore(string keyStore)
                {
                    command_ += " --J=-Dgemfire.ssl-keystore=" + keyStore;
                    return this;
                }

                public Server withSslKeyStorePassword(string keyStorePassword)
                {
                    command_ += " --J=-Dgemfire.ssl-keystore-password=" + keyStorePassword;
                    return this;
                }

                public Server withSslTrustStore(string trustStore)
                {
                    command_ += " --J=-Dgemfire.ssl-truststore=" + trustStore;
                    return this;
                }

                public Server withSslTrustStorePassword(string trustStorePassword)
                {
                    command_ += " --J=-Dgemfire.ssl-truststore-password=" + trustStorePassword;
                    return this;
                }

                public Server withSslEnableComponents(string components)
                {
                    command_ += " --J=-Dgemfire.ssl-enabled-components=" + components;
                    return this;
                }
            }

            public Server server()
            {
                return new Server(gfsh_);
            }

            public class Locator : Command
            {
                public Locator(Gfsh gfsh) : base(gfsh, "start locator")
                {
                }

                public Locator withName(string name)
                {
                    command_ += " --name=" + name;
                    return this;
                }

                public Locator withDir(string dir)
                {
                    command_ += " --dir=" + dir;
                    return this;
                }

                public Locator withBindAddress(string bindAddress)
                {
                    command_ += " --bind-address=" + bindAddress;
                    return this;
                }

                public Locator withPort(int port)
                {
                    command_ += " --port=" + port;
                    return this;
                }

                public Locator withJmxManagerPort(int jmxManagerPort)
                {
                    command_ += " --J=-Dgemfire.jmx-manager-port=" + jmxManagerPort;
                    return this;
                }

                public Locator withJmxManagerStart(bool start)
                {
                    command_ += " --J=-Dgemfire.jmx-manager-start=" + (start ? "true" : "false");
                    return this;
                }
                
                public Locator withHttpServicePort(int httpServicePort)
                {
                    command_ += " --http-service-port=" + httpServicePort;
                    return this;
                }

                public Locator withLogLevel(string logLevel)
                {
                    command_ += " --log-level=" + logLevel;
                    return this;
                }

                public Locator withMaxHeap(string maxHeap)
                {
                    command_ += " --max-heap=" + maxHeap;
                    return this;
                }

                public Locator withConnect(bool connect)
                {
                    command_ += " --connect=";
                    command_ += connect ? "true" : "false";
                    return this;
                }

                public Locator withSslKeyStore(string keyStore)
                {
                    command_ += " --J=-Dgemfire.ssl-keystore=" + keyStore;
                    return this;
                }

                public Locator withSslKeyStorePassword(string keyStorePassword)
                {
                    command_ += " --J=-Dgemfire.ssl-keystore-password=" + keyStorePassword;
                    return this;
                }

                public Locator withSslTrustStore(string trustStore)
                {
                    command_ += " --J=-Dgemfire.ssl-truststore=" + trustStore;
                    return this;
                }

                public Locator withSslTrustStorePassword(string trustStorePassword)
                {
                    command_ += " --J=-Dgemfire.ssl-truststore-password=" + trustStorePassword;
                    return this;
                }

                public Locator withSslEnableComponents(string components)
                {
                    command_ += " --J=-Dgemfire.ssl-enabled-components=" + components;
                    return this;
                }
            }

            public Locator locator()
            {
                return new Locator(gfsh_);
            }

            private Gfsh gfsh_;
        }

        public Start start()
        {
            return new Start(this);
        }

        public class Stop
        {
            public Stop(Gfsh gfsh)
            {
                gfsh_ = gfsh;
            }

            public class Locator : Command
            {
                public Locator(Gfsh gfsh) : base(gfsh, "stop locator")
                {
                }

                public Locator withName(string name)
                {
                    command_ += " --name=" + name;
                    return this;
                }

                public Locator withDir(string dir)
                {
                    command_ += " --dir=" + dir;
                    return this;
                }
            }

            public Locator locator()
            {
                return new Locator(gfsh_);
            }

            public class Server : Command
            {
                public Server(Gfsh gfsh) : base(gfsh, "stop server")
                {
                }

                public Server withName(string name)
                {
                    command_ += " --name=" + name;
                    return this;
                }

                public Server withDir(string dir)
                {
                    command_ += " --dir=" + dir;
                    return this;
                }
            }
            public Server server()
            {
                return new Server(gfsh_);
            }

            private Gfsh gfsh_;
        }

        public Stop stop()
        {
            return new Stop(this);
        }

        public class Create
        {
            public Create(Gfsh gfsh)
            {
                gfsh_ = gfsh;
            }

            public class Region : Command
            {
                public Region(Gfsh gfsh) : base(gfsh, "create region") { }

                public Region withName(string name)
                {
                    command_ += " --name=" + name;
                    return this;
                }

                public Region withType(string type)
                {
                    command_ += " --type=" + type;
                    return this;
                }
            }

            public Region region()
            {
                return new Region(gfsh_);
            }

            private Gfsh gfsh_;
        }
        public Create create()
        {
            return new Create(this);
        }

        public class Shutdown : Command
        {
            public Shutdown(Gfsh gfsh) : base(gfsh, "shutdown") { }

            public Shutdown withIncludeLocators(bool includeLocators)
            {
                command_ += " --include-locators=";
                command_ += includeLocators ? "true" : "false";
                return this;
            }
        }

        public Shutdown shutdown()
        {
            return new Shutdown(this);
        }

        public class Connect : Command
        {
            public Connect(Gfsh gfsh) : base(gfsh, "connect") { }

            public Connect withJmxManager(string jmxManagerAddress, int jmxManagerPort)
            {
                command_ += " --jmx-manager=" + jmxManagerAddress + "[" + jmxManagerPort.ToString() + "]";
                return this;
            }

            public Connect withUseSsl(bool enable)
            {
                command_ += " --use-ssl=" + (enable ? "true" : "false");
                return this;
            }

            public Connect withKeyStore(string keyStore)
            {
                command_ += " --key-store=" + keyStore;
                return this;
            }

            public Connect withKeyStorePassword(string keyStorePassword)
            {
                command_ += " --key-store-password=" + keyStorePassword;
                return this;
            }

            public Connect withTrustStore(string trustStore)
            {
                command_ += " --trust-store=" + trustStore;
                return this;
            }

            public Connect withTrustStorePassword(string trustStorePassword)
            {
                command_ += " --trust-store-password=" + trustStorePassword;
                return this;
            }
        }

        public Connect connect()
        {
            return new Connect(this);
        }

        public class ConfigurePdx : Command
        {
            public ConfigurePdx(Gfsh gfsh) : base(gfsh, "configure pdx") { }

            public ConfigurePdx withReadSerialized(bool readSerialized)
            {
                command_ += " --read-serialized=";
                command_ += readSerialized ? "true" : "false";
                return this;
            }
        }

        public ConfigurePdx configurePdx()
        {
            return new ConfigurePdx(this);
        }

        public class Deploy : Command
        {
            public Deploy(Gfsh gfsh) : base(gfsh, "deploy") { }

            public Deploy withJar(string fullPathToJar)
            {
                command_ += " --jar=" + fullPathToJar;
                return this;
            }

            public Deploy withDir(string fullPathToDir)
            {
                command_ += " --dir=" + fullPathToDir;
                return this;
            }

            public Deploy withGroup(string groupName)
            {
                command_ += " --group=" + groupName;
                return this;
            }
        }

        public Deploy deploy()
        {
            return new Deploy(this);
        }

        public class ExecuteFunction : Command
        {
            public ExecuteFunction(Gfsh gfsh) : base(gfsh, "execute function") { }

            public ExecuteFunction withId(string functionId)
            {
                command_ += " --id=" + functionId;
                return this;
            }

            public ExecuteFunction withMember(string memberName)
            {
                command_ += " --member=" + memberName;
                return this;
            }
        }

        public ExecuteFunction executeFunction()
        {
            return new ExecuteFunction(this);
        }

        private static int FreeTcpPort()
        {
            var tcpListner = new TcpListener(IPAddress.Loopback, 0);
            tcpListner.Start();
            var port = ((IPEndPoint)tcpListner.LocalEndpoint).Port;
            tcpListner.Stop();
            return port;
        }

        public abstract int execute(string cmd);
    }
}
