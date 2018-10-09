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
using System.IO;
using System.Diagnostics;

public class CacheXml : IDisposable
{
    public FileInfo File
    {
        get;
        private set;
    }

    public CacheXml(FileInfo template, int locatorPort, string regionName = "testRegion")
    {
        string content;

        using (var input = template.OpenText())
        {
            content = input.ReadToEnd();
        }

        content = content.Replace("LOCATOR_PORT", locatorPort.ToString());
        content = content.Replace("REGION_NAME", regionName);

        Debug.WriteLine(content);

        var tempFile = new FileInfo(Path.GetTempFileName()) { Attributes = FileAttributes.Temporary };

        // Set the Attribute property of this file to Temporary. 
        // Although this is not completely necessary, the .NET Framework is able 
        // to optimize the use of Temporary files by keeping them cached in memory.
        using (var output = new StreamWriter(tempFile.FullName))
        {
            output.Write(content);
        }

        File = tempFile;
    }

    public void Dispose()
    {
        File.Delete();
    }
}
