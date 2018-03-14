using System;
using System.Diagnostics;
using System.Net;
using System.IO;
using System.Net.Sockets;
using Xunit;

public class GemFireServer : IDisposable
{
    #region Properties/Fields

    public int LocatorPort { get; private set; }

    #endregion

    #region Public methods

    public GemFireServer(string regionName = "session", bool readSerialized = false)
    {
        try
        {
            //Clean up previous server dirs
            foreach (var dir in new DirectoryInfo(Environment.CurrentDirectory).GetDirectories())
            {
                dir.Delete(true);
            }
        }
        catch
        {
            //Ignored
        }

        LocatorPort = FreeTcpPort();
        var locatorJmxPort = FreeTcpPort();

        string readSerializedStr = readSerialized ? "--read-serialized=true" : "--read-serialized=false";

        var gfsh = new Process
        {
            StartInfo =
            {
                FileName = "\\bin\\gfsh.bat",
                Arguments = " -e \"start locator --bind-address=localhost --port=" + LocatorPort +
                            " --J=-Dgemfire.jmx-manager-port=" + locatorJmxPort + " --http-service-port=0\"" +
                            " -e \"connect --locator=localhost[" + LocatorPort + "]\"" +
                            " -e \"configure pdx " + readSerializedStr + "\"" +
                            " -e \"start server --bind-address=localhost --server-port=0 --log-level=all\"" +
                            " -e \"create region --name=" + regionName + " --type=PARTITION\"" +
                            " -e \"create region --name=testRegion1 --type=PARTITION\"" +
                            " -e \"deploy --jar=..\\..\\SessionStateStoreFunctions\\SessionStateStoreFunctions.jar\"",
                WindowStyle = ProcessWindowStyle.Hidden,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            }
        };

        gfsh.OutputDataReceived += (sender, args) =>
        {
            if (null != args.Data)
                Debug.WriteLine("GemFireServer: " + args.Data);
        };

        gfsh.ErrorDataReceived += (sender, args) =>
        {
            if (null != args.Data)
                Debug.WriteLine("GemFireServer: ERROR: " + args.Data);
        };

        gfsh.Start();
        gfsh.BeginOutputReadLine();
        gfsh.BeginErrorReadLine();
        gfsh.WaitForExit(60000 * 10);

        Debug.WriteLine("GemFireServer Start: gfsh.HasExited = {0}, gfsh.ExitCode = {1}",
            gfsh.HasExited, gfsh.ExitCode);

        Assert.True(gfsh.HasExited);
        Assert.Equal(0, gfsh.ExitCode);
    }

    public void Dispose()
    {
        try
        {
            var gfsh = new Process
            {
                StartInfo =
                {
                    FileName = "c:\\gemfire\\bin\\gfsh.bat",
                    Arguments = "-e \"connect --locator=localhost[" + LocatorPort +
                                "]\" -e \"shutdown --include-locators true\" ",
                    WindowStyle = ProcessWindowStyle.Hidden,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                }
            };

            gfsh.OutputDataReceived += (sender, args) =>
            {
                if (null != args.Data)
                    Debug.WriteLine("GemFireServer: " + args.Data);
            };

            gfsh.ErrorDataReceived += (sender, args) =>
            {
                if (null != args.Data)
                    Debug.WriteLine("GemFireServer: ERROR: " + args.Data);
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
        TcpListener l = new TcpListener(IPAddress.Loopback, 0);
        l.Start();
        var port = ((IPEndPoint)l.LocalEndpoint).Port;
        l.Stop();
        return port;
    }

    #endregion
}
