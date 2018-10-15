﻿/*
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
using System.IO;
using System.Reflection;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class TestBase
    {
        private const int MaxAllowedDirectoryCharacters = 15;

        public void CleanTestCaseDirectory(string directory)
        {
            if (Directory.Exists(directory))
            {
                Directory.Delete(directory, true);
            }
        }

        public string CreateTestCaseDirectoryName()
        {
            var st = new StackTrace();
            var sf = st.GetFrame(1);
            var currentMethod = sf.GetMethod();
            var dirName = currentMethod.Name;

            if (dirName.Length > MaxAllowedDirectoryCharacters)
            {
                dirName = dirName.Substring(0, MaxAllowedDirectoryCharacters);
            }
            return dirName;
        }
    }
}
