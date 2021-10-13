/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.Diagnostics;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    public class GfshExecute : Gfsh
    {
        private String connectionCommand_ = null;
        private ITestOutputHelper output;

        public ITestOutputHelper Output
        {
          get { return output; }
          set { output = value; }
        }


    public GfshExecute(ITestOutputHelper output)
        {
            //Output = output;
            this.output = output;
    }

        private void ExtractConnectionCommand(String command)
        {
            if (command.StartsWith("connect"))
            {
                connectionCommand_ = command;
            }
            else if (command.StartsWith("start locator"))
            {
                if (command.Contains("--connect=false"))
                {
                    return;
                }

                var jmxManagerHost = "localhost";
                var jmxManagerPort = "1099";

                var jmxManagerHostRegex = new Regex(@"\bbind-address=([^\s])\b");
                var jmxManagerHostMatch = jmxManagerHostRegex.Match(command);

                if (jmxManagerHostMatch.Success)
                {
                    jmxManagerHost = jmxManagerHostMatch.Groups[1].Value;
                }

                var jmxManagerPortRegex = new Regex(@"\bjmx-manager-port=(\d+)\b");
                var jmxManagerPortMatch = jmxManagerPortRegex.Match(command);
                if (jmxManagerPortMatch.Success)
                {
                    jmxManagerPort = jmxManagerPortMatch.Groups[1].Value;
                }

                connectionCommand_ = new Connect(this).withJmxManager(jmxManagerHost, int.Parse(jmxManagerPort)).ToString();
            }

        }

        public override int execute(string cmd)
        {

            var commands = new List<string>();

            if (null != connectionCommand_)
            {
                commands.Add("-e");
                commands.Add(connectionCommand_);
            }

            commands.Add("-e");
            commands.Add(cmd);

            // TODO escape commands
            var fullCmd = "\"" + string.Join("\" \"", commands) + "\"";

            using var gfsh = new Process
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
                    WriteLine("GfshExecute: " + args.Data);
                }
            };

            gfsh.ErrorDataReceived += (sender, args) =>
            {
                if (args.Data != null)
                {
                    WriteLine("GfshExecute: ERROR: " + args.Data);
                }
            };

            gfsh.Start();
            gfsh.BeginOutputReadLine();
            gfsh.BeginErrorReadLine();
            if (gfsh.WaitForExit(60000))
            {
                WriteLine("GeodeServer Start: gfsh.HasExited = {0}, gfsh.ExitCode = {1}",
                    gfsh.HasExited,
                    gfsh.ExitCode);
            }
            else
            {
                WriteLine("GeodeServer Start: gfsh failed to exit, force killing.");
                KillAndIgnore(gfsh);
            }
            CancelErrorReadAndIgnore(gfsh);
            CancelOutputReadAndIgnore(gfsh);

            ExtractConnectionCommand(cmd);

            return gfsh.ExitCode;
        }

        private static void CancelOutputReadAndIgnore(Process gfsh)
        {
            try
            {
                gfsh.CancelOutputRead();
            }
            catch
            {
                // ignored
            }
        }

        private static void CancelErrorReadAndIgnore(Process gfsh)
        {
            try
            {
                gfsh.CancelErrorRead();
            }
            catch
            {
                // ignored
            }
        }

        private static void KillAndIgnore(Process gfsh)
        {
            try
            {
                gfsh.Kill();
            }
            catch
            {
                // ignored
            }
        }

        private void WriteLine(string format, params object[] args)
        {
            if (null == output)
            {
                Debug.WriteLine(format, args);
            }
            else
            {
                output.WriteLine(format, args);
            }
        }

        private void WriteLine(string message)
        {
            if (null == output)
            {
                Debug.WriteLine(message);
            }
            else
            {
                output.WriteLine(message);
            }
        }
    }
}
