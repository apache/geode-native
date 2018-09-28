using System;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class GfshExecuteTest : IDisposable
    {
        public void Dispose()
        {

        }

        [Fact]
        public void GfshExecuteStartLocatorTest()
        {
            GfshExecute gfsh = new GfshExecute();
            var exitCode = gfsh.start().locator().withPort(gfsh.LocatorPort).execute();

            Assert.Equal(exitCode, 0);
        }

        [Fact]
        public void GfshExecuteStartServerTest()
        {
            GfshExecute gfsh = new GfshExecute();
            var locatorExitCode = gfsh.start().locator().withPort(gfsh.LocatorPort).execute();
            var serverExitCode = gfsh.start().server().withLocators("localhost[" + gfsh.LocatorPort + "]").execute();
            var shutdownExitCode = gfsh.shutdown().withIncludeLocators(true).execute();

            Assert.Equal(locatorExitCode, 0);
            Assert.Equal(serverExitCode, 0);
            Assert.Equal(shutdownExitCode, 0);
        }
    }
}
