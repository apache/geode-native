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

#include "begin_native.hpp"
#include <chrono>
#include "end_native.hpp"
#include "TimeUtils.hpp"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

using namespace std::chrono;

using namespace Apache::Geode::Client;

namespace cliunittests
{
  [TestClass]
  public ref class TimeUtilsTests
  {
  private:
    TestContext^ testContextInstance;

  public: 
    /// <summary>
    ///Gets or sets the test context which provides
    ///information about and functionality for the current test run.
    ///</summary>
    property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
    {
      Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
      {
        return testContextInstance;
      }
      System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
      {
        testContextInstance = value;
      }
    };

    #pragma region Additional test attributes
    //
    //You can use the following additional attributes as you write your tests:
    //
    //Use ClassInitialize to run code before running the first test in the class
    //[ClassInitialize()]
    //static void MyClassInitialize(TestContext^ testContext) {};
    //
    //Use ClassCleanup to run code after all tests in a class have run
    //[ClassCleanup()]
    //static void MyClassCleanup() {};
    //
    //Use TestInitialize to run code before running each test
    //[TestInitialize()]
    //void MyTestInitialize() {};
    //
    //Use TestCleanup to run code after each test has run
    //[TestCleanup()]
    //void MyTestCleanup() {};
    //
    #pragma endregion 

    [TestMethod]
    void TimePointToDateTime()
    {
      auto testSystemEpoch = DateTime(1970, 1, 1, 0, 0, 0, 0, gcnew System::Globalization::GregorianCalendar(), DateTimeKind::Utc);
      Assert::AreEqual(621355968000000000, testSystemEpoch.Ticks);

      auto systemEpoch = TimeUtils::TimePointToDateTime(system_clock::time_point());
      Assert::AreEqual(621355968000000000, systemEpoch.Ticks);

      auto systemNextDay = system_clock::time_point() + hours(24);
      auto netNextDay = testSystemEpoch.AddDays(1);
      auto convertedNextDay = TimeUtils::TimePointToDateTime(systemNextDay);
      Assert::AreEqual(netNextDay, convertedNextDay);
    }

    [TestMethod]
    void DateTimeToTimePoint()
    {
      const auto netEpoch = TimeUtils::DateTimeToTimePoint(DateTime(0));
      Assert::AreEqual(-621355968000000000, ticks{netEpoch.time_since_epoch()}.count());
    }

  };
}
