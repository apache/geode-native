# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

choco install jdk8 -confirm
choco install cmake.portable -confirm
choco install git.install -confirm
choco install activeperl -confirm
#Restore this line when Chocolatey Doxygen install gets fixed (2018-12-06)
#choco install doxygen.install --allowEmptyChecksums -confirm
choco install dogtail.dotnet3.5sp1 -confirm
choco install nunit.install --version 2.6.4 -confirm
choco install netfx-4.5.2-devpack --allowEmptyChecksums -confirm
choco install nsis -confirm
choco install patch -confirm
choco install gnuwin32-coreutils.portable -confirm
choco install nuget.commandline -confirm
