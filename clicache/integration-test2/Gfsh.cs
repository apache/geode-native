using System;
using System.Diagnostics;
using System.Net;
using System.IO;
using System.Net.Sockets;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
    public abstract class Gfsh
    {
        public string LocatorBindAddress { get; set; }
        public int LocatorPort { get; private set; }
        public int JmxManagerPort { get; private set; }
        public int HttpServicePort { get; private set; }
        public int ServerPort { get; private set; }
        public string ServerBindAddress { get; private set; }
        public string Connection { get; private set; }

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
                //return (T)new T().SetGfsh(gfsh_); // .parse(gfsh_.execute(command_));
                return gfsh_.execute(command_);
            }

            public override string ToString()
            {
                return "Command: " + command_;
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
                    gfsh_.ServerBindAddress = bindAddress;
                    return this;
                }

                public Server withPort(int port)
                {
                    gfsh_.ServerPort = port;
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
                    gfsh_.LocatorBindAddress = bindAddress;
                    return this;
                }

                public Locator withPort(int port)
                {
                    gfsh_.LocatorPort = port;
                    return this;
                }
                public Locator withJmxManagerPort(short jmxManagerPort)
                {
                    gfsh_.JmxManagerPort = jmxManagerPort;
                    return this;
                }

                public Locator withHttpServicePort(short httpServicePort)
                {
                    command_ += " --http-service-port=" + Convert.ToString(httpServicePort);
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
            public Shutdown(Gfsh gfsh) : base(gfsh, "shutdown") {}

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

        private static string defaultBindAddress = "localhost";
        private static int defaultHttpServicePort = 0;
        public Gfsh()
        {
            LocatorBindAddress = defaultBindAddress;
            HttpServicePort = defaultHttpServicePort;
            ServerBindAddress = defaultBindAddress;
            Connection = String.Empty; 

            //TODO: May need a map or something to prevent port collisions
            LocatorPort = FreeTcpPort();
            JmxManagerPort = FreeTcpPort();
            ServerPort = FreeTcpPort();
        }

        private static int FreeTcpPort()
        {
            var tcpListner = new TcpListener(IPAddress.Loopback, 0);
            tcpListner.Start();
            var port = ((IPEndPoint) tcpListner.LocalEndpoint).Port;
            tcpListner.Stop();
            return port;
        }

        public abstract int execute(string cmd);
    }
}
