using System;
using System.Diagnostics;
using System.Net;
using System.IO;
using System.Net.Sockets;
using Xunit;

public class GeodeServer : IDisposable
{
  #region Properties/Fields

  public int LocatorPort { get; private set; }
  public int LocatorJmxPort { get; private set; }

  private bool _useSsl;

  #endregion

  #region Public methods

  public GeodeServer(string regionName = "testRegion", bool readSerialized = false, bool useSsl = false)
  {
    _useSsl = useSsl;
    try
    {
      //Clean up previous server dirs
      foreach (var dir in new DirectoryInfo(Environment.CurrentDirectory).GetDirectories())
      {
        if (!dir.Name.Equals("ServerSslKeys", StringComparison.OrdinalIgnoreCase)
            && !dir.Name.Equals("ClientSslKeys", StringComparison.OrdinalIgnoreCase))
        {
          dir.Delete(true);
        }
      }
    }
    catch
    {
      //Ignored
    }

    LocatorPort = FreeTcpPort();
    LocatorJmxPort = FreeTcpPort();

    var readSerializedStr = readSerialized ? "--read-serialized=true" : "--read-serialized=false";

    Process gfsh;
    if (_useSsl)
    {
      gfsh = new Process
      {
        StartInfo =
        {
          FileName = Config.GeodeGfsh,
          Arguments = " -e \"start locator --bind-address=localhost --port=" + LocatorPort +
                      " --J=-Dgemfire.jmx-manager-port=" + LocatorJmxPort +
                      " --http-service-port=0 --connect=false --J=-Dgemfire.ssl-enabled-components=locator,jmx" +
                      " --J=-Dgemfire.ssl-keystore=" + Environment.CurrentDirectory +
                      "/ServerSslKeys/server_keystore.jks --J=-Dgemfire.ssl-keystore-password=gemstone" +
                      " --J=-Dgemfire.ssl-truststore=" + Environment.CurrentDirectory +
                      "/ServerSslKeys/server_truststore.jks --J=-Dgemfire.ssl-truststore-password=gemstone\"" +
                      " -e \"connect --locator=localhost[" + LocatorPort + "] --use-ssl --key-store=" +
                      Environment.CurrentDirectory +
                      "/ServerSslKeys/server_keystore.jks --key-store-password=gemstone " +
                      " --trust-store=" + Environment.CurrentDirectory +
                      "/ServerSslKeys/server_truststore.jks --trust-store-password=gemstone\"" +
                      " -e \"configure pdx " + readSerializedStr + "\"" +
                      " -e \"start server --bind-address=localhost --server-port=0 --log-level=all" +
                      " --J=-Dgemfire.ssl-enabled-components=server,locator,jmx --J=-Dgemfire.ssl-keystore=" +
                      Environment.CurrentDirectory + "/ServerSslKeys/server_keystore.jks" +
                      " --J=-Dgemfire.ssl-keystore-password=gemstone --J=-Dgemfire.ssl-truststore=" +
                      Environment.CurrentDirectory +
                      "/ServerSslKeys/server_truststore.jks --J=-Dgemfire.ssl-truststore-password=gemstone\"" +
                      " -e \"create region --name=" + regionName + " --type=PARTITION\"" +
                      " -e \"create region --name=testRegion1 --type=PARTITION\"" +
                      " -e \"create region --name=cqTestRegion --type=REPLICATE\"",
          WindowStyle = ProcessWindowStyle.Hidden,
          UseShellExecute = false,
          RedirectStandardOutput = true,
          RedirectStandardError = true,
          CreateNoWindow = false
        }
      };
    }
    else
    {
      gfsh = new Process
      {
        StartInfo =
        {
          FileName = Config.GeodeGfsh,
          Arguments = " -e \"start locator --bind-address=localhost --port=" + LocatorPort +
                      " --J=-Dgemfire.jmx-manager-port=" + LocatorJmxPort + " --http-service-port=0" + "\"" +
                      " -e \"start server --bind-address=localhost --server-port=0\"" +
                      " -e \"create region --name=" + regionName + " --type=PARTITION\"" +
                      " -e \"create region --name=testRegion1 --type=PARTITION\"" +
                      " -e \"create region --name=cqTestRegion --type=REPLICATE\"",
          WindowStyle = ProcessWindowStyle.Hidden,
          UseShellExecute = false,
          RedirectStandardOutput = true,
          RedirectStandardError = true,
          CreateNoWindow = true
        }
      };
    }

    gfsh.OutputDataReceived += (sender, args) =>
    {
      if (null != args.Data)
        Debug.WriteLine("GeodeServer: " + args.Data);
    };

    gfsh.ErrorDataReceived += (sender, args) =>
    {
      if (null != args.Data)
        Debug.WriteLine("GeodeServer: ERROR: " + args.Data);
    };


    gfsh.Start();
    gfsh.BeginOutputReadLine();
    gfsh.BeginErrorReadLine();
    if (gfsh.WaitForExit(60000))
    {
      Debug.WriteLine("GeodeServer Start: gfsh.HasExited = {0}, gfsh.ExitCode = {1}",
        gfsh.HasExited, gfsh.ExitCode);
    }
    else
    {
      Debug.WriteLine("GeodeServer Start: gfsh failed to exit, force killing.");
      try
      {
        gfsh.Kill();
      }
      catch
      {
        // ignored
      }
    }
  }

  public void Dispose()
  {
    try
    {
      Process gfsh;

      if (_useSsl)
      {
        gfsh = new Process
        {
          StartInfo =
            {
              FileName = Config.GeodeGfsh,
              Arguments = "-e \"connect --jmx-manager=localhost[" + LocatorJmxPort  + "] --use-ssl --key-store=" +
                          Environment.CurrentDirectory +
                          "/ServerSslKeys/server_keystore.jks --key-store-password=gemstone --trust-store=" +
                          Environment.CurrentDirectory +
                          "/ServerSslKeys/server_truststore.jks --trust-store-password=gemstone\" -e \"shutdown --include-locators true\" ",
              WindowStyle = ProcessWindowStyle.Hidden,
              UseShellExecute = false,
              RedirectStandardOutput = true,
              RedirectStandardError = true,
              CreateNoWindow = true
            }
        };
      }
      else
      {
        gfsh = new Process
        {
          StartInfo =
            {
              FileName = Config.GeodeGfsh,
              Arguments = "-e \"connect --jmx-manager=localhost[" + LocatorJmxPort +
                          "]\" -e \"shutdown --include-locators true\" ",
              WindowStyle = ProcessWindowStyle.Hidden,
              UseShellExecute = false,
              RedirectStandardOutput = true,
              RedirectStandardError = true,
              CreateNoWindow = true
            }
        };
      }

      gfsh.OutputDataReceived += (sender, args) =>
      {
        if (null != args.Data)
          Debug.WriteLine("GeodeServer: " + args.Data);
      };

      gfsh.ErrorDataReceived += (sender, args) =>
      {
        if (null != args.Data)
          Debug.WriteLine("GeodeServer: ERROR: " + args.Data);
      };

      gfsh.Start();
      gfsh.BeginOutputReadLine();
      gfsh.BeginErrorReadLine();
      gfsh.WaitForExit(30000);
    }
    catch
    {
      // ignored
    }
  }

  #endregion

  #region Private Methods

  private static int FreeTcpPort()
  {
    var tcpListner = new TcpListener(IPAddress.Loopback, 0);
    tcpListner.Start();
    var port = ((IPEndPoint) tcpListner.LocalEndpoint).Port;
    tcpListner.Stop();
    return port;
  }

  #endregion
}
