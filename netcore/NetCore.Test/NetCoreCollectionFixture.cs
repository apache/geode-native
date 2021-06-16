using System;
using Apache.Geode.NetCore;
using Xunit;

public class NetCoreCollectionFixture : IDisposable
{
    public NetCoreCollectionFixture()
    {
        client_ = new Client();
    }
    public void Dispose()
    {
        client_.Dispose();
    }

    Client client_;
}

[CollectionDefinition("Geode .net Core Collection")]
public class NetCoreCollection : ICollectionFixture<NetCoreCollectionFixture>
{
}
