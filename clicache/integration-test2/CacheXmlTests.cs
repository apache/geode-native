using System.IO;
using System.Threading;
using Xunit;

[Trait("Category", "Integration")]
public class CacheXmlTests
{
    [Fact]
    public void ConstructAndGenerate()
    {
        using (var gfs = new GeodeServer())
        {
            var template = new FileInfo("cache.xml");
            var cacheXml = new CacheXml(template, gfs);
            Assert.NotNull(cacheXml.File);
            Assert.True(cacheXml.File.Exists);

            using (var input = cacheXml.File.OpenText())
            {
                var content = input.ReadToEnd();
                Assert.True(content.Contains(gfs.LocatorPort.ToString()));
            }
        }
    }

    [Fact]
    public void DisposeAndCleanup()
    {
        using (var gfs = new GeodeServer())
        {
            FileInfo file;

            var template = new FileInfo("cache.xml");
            using (var cacheXml = new CacheXml(template, gfs))
            {
                Assert.NotNull(cacheXml.File);
                file = cacheXml.File;
                Assert.True(file.Exists);
            }

            file.Refresh();

            // File deletion via File.Delete (inside the file.Refresh() call)
            // is not synchronous so we need to potentially wait until the file 
            // has been deleted here
            Assert.True(SpinWait.SpinUntil(() => !file.Exists, 10000));
        }
    }
}
