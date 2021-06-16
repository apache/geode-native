using System;

namespace Apache 
{
    namespace Geode
    {
        namespace NetCore
        {
            public abstract class GeodeNativeObject : IDisposable
            {
                private bool _disposed = false;
                protected IntPtr _containedObject;
                
                
                public void Dispose()
                {
                    Dispose(true);
                    GC.SuppressFinalize(this);
                }

                public IntPtr ContainedObject {
                    get 
                    {
                        return _containedObject;
                    }
                }
                
                protected virtual void Dispose(bool disposing)
                {
                    if (_disposed)
                    {
                        return;
                    }

                    if (disposing)
                    {
                        DestroyContainedObject();
                        _containedObject = IntPtr.Zero;
                    }

                    _disposed = true;
                }

                protected abstract void DestroyContainedObject();
            }
        }
    }
}
