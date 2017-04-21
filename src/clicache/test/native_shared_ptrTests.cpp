#include <memory>
#include <functional>
#include <native_shared_ptr.hpp>
#include "Utils.hpp"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

using namespace Apache::Geode::Client;

namespace cliunittests
{

  class NativeTestClass {
  public:
    int i;
    NativeTestClass(int i) : i(i) {};
  };

  class DestructNativeTestClass {
  public:
    int i;
    DestructNativeTestClass(int i, bool& destructorCalled) : i(i), destructorCalled(destructorCalled) {};
    ~DestructNativeTestClass() { destructorCalled = true; }
    void GCCollectAndAssertDestructorCalledEquals(bool expected, const bool& destructorCalled)
    {
      Utils::GCCollectAndWait();
      Assert::AreEqual(expected, destructorCalled);
    }
 

  private:
    bool& destructorCalled;
  };


	[TestClass]
	public ref class native_shared_ptrTests
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
		void NullptrInstance()
		{
      auto p = gcnew native_shared_ptr<NativeTestClass>(__nullptr);
      
      Assert::IsNotNull(p);
      Assert::IsTrue(__nullptr == p->get());
		};

    [TestMethod]
		void AnInstance()
		{
      auto n = std::make_shared<NativeTestClass>(1);
      auto p = gcnew native_shared_ptr<NativeTestClass>(n);

      Assert::IsNotNull(p);
      Assert::IsFalse(__nullptr == p->get());
      Assert::AreEqual(1, p->get()->i);

      auto x = p->get_shared_ptr();
      Assert::IsTrue(x == n);
		};

    [TestMethod]
    void DestructorCalledAfterGC()
    {
      bool destructorCalled = false;
      auto p = gcnew native_shared_ptr<DestructNativeTestClass>(std::make_shared<DestructNativeTestClass>(1, destructorCalled));

      // p eligible for GC
      Utils::GCCollectAndWait();

      Assert::IsTrue(destructorCalled);
    }

    [TestMethod]
    void DestructorCalledAfterAllSharedPtrDescopesButNotAfterNativeSharePtrIsDestructed()
    {
      bool destructorCalled = false;
      {
        auto n = std::make_shared<DestructNativeTestClass>(1, destructorCalled);
        auto p = gcnew native_shared_ptr<DestructNativeTestClass>(n);

        // p eligible for GC
        Utils::GCCollectAndWait();

        // n should still have a ref on our native pointer, so native not deleted
        Assert::IsFalse(destructorCalled);
      }

      // n has descoped so ref on native pointer is zero, should be deleted
      Assert::IsTrue(destructorCalled);
    }


    [TestMethod]
    void DestructorCalledAfterAllReferencesReleased()
    {
      bool destructorCalled = false;
      auto p = gcnew native_shared_ptr<DestructNativeTestClass>(std::make_shared<DestructNativeTestClass>(1, destructorCalled));
      auto q = p;

      // only p eligible for collection
      Utils::GCCollectAndWait();

      Assert::IsFalse(destructorCalled);
     
      // keeps q ineligible for collection
      GC::KeepAlive(q);

      // q eligible for collection
      Utils::GCCollectAndWait();

      Assert::IsTrue(destructorCalled);
    }

    native_shared_ptr<DestructNativeTestClass>^ make_shared(int i, bool& destructorCalled) {
      return gcnew native_shared_ptr<DestructNativeTestClass>(std::make_shared<DestructNativeTestClass>(1, destructorCalled));
    }

    [TestMethod]
    void GCCollectsWhileExecutingOnNativeClass()
    {
      bool destructorCalled = false;
      auto p = make_shared(1, destructorCalled);

      p->get()->GCCollectAndAssertDestructorCalledEquals(true, destructorCalled);
    }

    [TestMethod]
    void GCCollectsAfterExecutingOnNativeClass()
    {
      bool destructorCalled = false;
      auto p = make_shared(1, destructorCalled);

      p->get()->GCCollectAndAssertDestructorCalledEquals(false, destructorCalled);
      // keeps p ineligible for collection
      GC::KeepAlive(p);

      // p eligible for collection
      Utils::GCCollectAndWait();
      Assert::IsTrue(destructorCalled);
    }

  };
}
