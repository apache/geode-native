using System.Collections.Generic;

namespace Apache 
{
    namespace Geode
    {
        namespace NetCore
        {
            public interface IAuthInitialize
            {
                Dictionary<string, string> GetCredentials();
                void Close();
            }
        }
    }
}