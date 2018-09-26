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
            public void execute()
            {
                //return (T)new T().SetGfsh(gfsh_); // .parse(gfsh_.execute(command_));
                gfsh_.execute(command_);
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
                    command_ += " --bind-address=" + bindAddress;
                    return this;
                }

                public Server withPort(short port)
                {
                    command_ += " --port=" + Convert.ToString(port);
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
                    command_ += " --bind-address=" + bindAddress;
                    return this;
                }

                public Locator withPort(short port)
                {
                    command_ += " --port=" + Convert.ToString(port);
                    return this;
                }
                public Locator withJmxManagerPort(short jmxManagerPort)
                {
                    command_ +=
                        " --J=-Dgemfire.jmx-manager-port=" + Convert.ToString(jmxManagerPort);
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

        public class Connect : Command
        {
            public Connect(Gfsh gfsh) : base(gfsh, "connect") {}

            public Connect withJmxManager(string jmxManager)
            {
                command_ += " --jmx-manager=" + jmxManager;
                return this;
            }
        }

        public Connect connect()
        {
            return new Connect(this);
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

        public Gfsh()
        {

        }

        public abstract void execute(string cmd);
    }
}
