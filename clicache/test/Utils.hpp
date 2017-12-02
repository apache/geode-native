#pragma once


using namespace System;

namespace cliunittests
{
  private ref class Utils {
  internal:
    static void GCCollectAndWait() {
      GC::Collect();
      GC::WaitForPendingFinalizers();
    }
  };
}