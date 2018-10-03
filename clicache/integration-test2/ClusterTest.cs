using System;
using System.Diagnostics;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class ClusterTest : IDisposable
    {
        public void Dispose()
        {

        }

        [Fact]
        public void ClusterStartTest()
        {
            using (var cluster = new Cluster("foo", 1, 1))
            {
                Assert.True(cluster.Start());
            }
        }

        [Fact]
        public void ClusterStartWithTwoServersTest()
        {
            using (var cluster = new Cluster("bar", 1, 2))
            {
                Assert.True(cluster.Start());
            }
        }
    }
}
