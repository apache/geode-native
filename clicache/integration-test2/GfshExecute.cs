using System;
using System.Diagnostics;
using System.Net;
using System.IO;
using System.Net.Sockets;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
    public class GfshExecute : Gfsh
    {
        public GfshExecute()
        {
        }
        public override void Dispose()
        {
        }

        private static string startLocator = "start locator";
        private static string startServer = "start server";

        private string buildStartLocatorCommand(string options)
        {
            string locatorCmd = startLocator;
            locatorCmd += " --port=" + LocatorPort;
            locatorCmd += " --bind-address=" + LocatorBindAddress;
            locatorCmd += " --J=-Dgemfire.jmx-manager-port=" + JmxManagerPort + " ";
            locatorCmd += " --J=-Dgemfire.jmx-manager-start=true";
            locatorCmd += options;
            return locatorCmd;
        }

        private string buildStartServerCommand(string options)
        {
            string serverCmd = "-e \"connect --jmx-manager=" + LocatorBindAddress
                + "[" + JmxManagerPort + "]\" -e \"" + startServer;
            serverCmd += " --bind-address=" + ServerBindAddress;
            serverCmd += options + "\"";
            return serverCmd;
        }

        private string buildConnectAndExecuteString(string options)
        {
            return "-e \"connect --jmx-manager=" + LocatorBindAddress
                + "[" + JmxManagerPort + "]\" -e \"" + options + "\"";
        }

        private string BuildFullCommandString(string baseCmd)
        {
            string fullCmd;

            if (baseCmd.IndexOf(startLocator) == 0)
            {
                fullCmd = buildStartLocatorCommand(baseCmd.Substring(startLocator.Length));
            }
            else if (baseCmd.IndexOf(startServer) == 0)
            {
                fullCmd = buildStartServerCommand(baseCmd.Substring(startServer.Length));
            }
            else
            {
                fullCmd = buildConnectAndExecuteString(baseCmd);
            }

            return fullCmd;
        }

        public override int execute(string cmd)
        {
            string fullCmd = BuildFullCommandString(cmd);

            Process gfsh = new Process
            {
                StartInfo =
                {
                    FileName = Config.GeodeGfsh,
                    Arguments = fullCmd,
                    WindowStyle = ProcessWindowStyle.Hidden,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = false
                }
            };

            gfsh.OutputDataReceived += (sender, args) =>
            {
                if (args.Data != null)
                {
                    Debug.WriteLine("GfshExecute: " + args.Data);
                }
            };

            gfsh.ErrorDataReceived += (sender, args) =>
            {
                if (args.Data != null)
                {
                    Debug.WriteLine("GfshExecute: ERROR: " + args.Data);
                }
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

            return gfsh.ExitCode;
        }
    }
}
