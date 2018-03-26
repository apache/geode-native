using Xunit;

[Trait("Category", "Integration")]
public class GemFireServerTest
{
    [Fact]
    public void Start()
    {
        var gfs = new GeodeServer();
        Assert.NotNull(gfs);
        Assert.NotEqual(0, gfs.LocatorPort);
        gfs.Dispose();
    }

    [Fact]
    public void StartTwo()
    {
        var gfs1 = new GeodeServer();
        Assert.NotNull(gfs1);
        Assert.NotEqual(0, gfs1.LocatorPort);

        var gfs2 = new GeodeServer();
        Assert.NotNull(gfs2);
        Assert.NotEqual(0, gfs2.LocatorPort);

        Assert.NotEqual(gfs1.LocatorPort, gfs2.LocatorPort);
        gfs1.Dispose();
        gfs2.Dispose();
    }
}
