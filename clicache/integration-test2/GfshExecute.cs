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
        public override void execute(string cmd)
        {
            System.Diagnostics.Debug.WriteLine("Executing: " + cmd);
        }
    }
}
