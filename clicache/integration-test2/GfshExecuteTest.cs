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
            using (GfshExecute gfsh = new GfshExecute())
            {
                try
                {
                    Assert.Equal(gfsh.start()
                        .locator()
                        .withHttpServicePort(0)
                        .withPort(gfsh.LocatorPort)
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh.shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }

        [Fact]
        public void GfshExecuteStartServerTest()
        {
            using (GfshExecute gfsh = new GfshExecute())
            {
                try
                {
                    Assert.Equal(gfsh.start()
                        .locator()
                        .withHttpServicePort(0)
                        .withPort(gfsh.LocatorPort)
                        .execute(), 0);
                    Assert.Equal(gfsh.start()
                        .server()
                        .withLocators("localhost[" + gfsh.LocatorPort + "]")
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh.shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }
    }
}
