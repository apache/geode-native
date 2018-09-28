using System;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class GfshTest : IDisposable
    {
        public void Dispose()
        {
        
        }

        [Fact]
        public void InstantiateGfshClasses()
        {
            Gfsh gfsh = new GfshExecute();
            Gfsh.Start start = gfsh.start();
            Assert.NotNull(start);

            Gfsh.Stop stop = gfsh.stop();
            Assert.NotNull(stop);

            Gfsh.Create create = gfsh.create();
            Assert.NotNull(create);

            Gfsh.Shutdown shutdown = gfsh.shutdown();
            Assert.NotNull(shutdown);
        }

        [Fact]
        public void StartLocatorStrings()
        {
            Gfsh.Start.Locator locator = new GfshExecute()
                .start()
                .locator();
            string s = locator.ToString();
            Assert.True(s.Equals("Command: start locator"));

            locator = new GfshExecute().start().locator()
                .withName("name")
                .withDir("dir")
                .withBindAddress("address")
                .withPort(420)
                .withJmxManagerPort(1111)
                .withHttpServicePort(2222)
                .withLogLevel("fine")
                .withMaxHeap("someHugeAmount");
            s = locator.ToString();
            Assert.True(s.Equals("Command: start locator --name=name --dir=dir " +
                    "--bind-address=address --port=420 " +
                    "--J=-Dgemfire.jmx-manager-port=1111 --http-service-port=" +
                    "2222 --log-level=fine --max-heap=someHugeAmount"));
        }

        [Fact]
        public void StartServerStrings()
        {
            Gfsh.Start.Server server = new GfshExecute()
                .start()
                .server();
            string s = server.ToString();
            Assert.True(s.Equals("Command: start server"));

            server = new GfshExecute()
                .start()
                .server()
                .withName("server")
                .withDir("someDir")
                .withBindAddress("someAddress")
                .withPort(1234)
                .withLocators("someLocator")
                .withLogLevel("debug")
                .withMaxHeap("1.21gigabytes");
            s = server.ToString();
            Assert.True(s.Equals("Command: start server --name=server " +
                "--dir=someDir --bind-address=someAddress --port=1234 " +
                "--locators=someLocator --log-level=debug " +
                "--max-heap=1.21gigabytes"));
        }

        [Fact]
        public void StopLocatorStrings()
        {
            Gfsh.Stop.Locator locator = new GfshExecute()
                .stop()
                .locator();
            string s = locator.ToString();
            Assert.True(s.Equals("Command: stop locator"));

            locator = new GfshExecute().stop().locator()
                .withName("name")
                .withDir("dir");
            s = locator.ToString();
            Assert.True(s.Equals("Command: stop locator --name=name --dir=dir"));
        }

        [Fact]
        public void StopServerStrings()
        {
            Gfsh.Stop.Server server = new GfshExecute()
                .stop()
                .server();
            string s = server.ToString();
            Assert.True(s.Equals("Command: stop server"));

            server = new GfshExecute()
                .stop()
                .server()
                .withName("server")
                .withDir("someDir");
            s = server.ToString();
            Assert.True(s.Equals("Command: stop server --name=server --dir=someDir"));
        }

        [Fact]
        public void CreateRegionStrings()
        {
            Gfsh.Create.Region region = new GfshExecute()
                .create()
                .region();
            string s = region.ToString();
            Assert.True(s.Equals("Command: create region"));

            region = new GfshExecute()
                .create()
                .region()
                .withName("region")
                .withType("PARTITION");
            s = region.ToString();
            Assert.True(s.Equals("Command: create region --name=region --type=PARTITION"));
        }

        [Fact]
        public void ShutdownStrings()
        {
            Gfsh.Shutdown shutdown = new GfshExecute()
                .shutdown();
            string s = shutdown.ToString();
            Assert.True(s.Equals("Command: shutdown"));

            shutdown = new GfshExecute()
                .shutdown()
                .withIncludeLocators(true);
            s = shutdown.ToString();
            Assert.True(s.Equals("Command: shutdown --include-locators=true"));

            shutdown = new GfshExecute()
                .shutdown()
                .withIncludeLocators(false);
            s = shutdown.ToString();
            Assert.True(s.Equals("Command: shutdown --include-locators=false"));
        }
    }
}
