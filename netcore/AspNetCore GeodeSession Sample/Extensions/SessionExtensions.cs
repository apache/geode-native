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
using System.Text.Json;
using Microsoft.AspNetCore.Http;

namespace Web.Extensions {
#region snippet1
  public static class SessionExtensions {
    public static void Set<T>(this ISession session, string key, T value) {
      session.SetString(key, JsonSerializer.Serialize(value));
    }

    public static T Get<T>(this ISession session, string key) {
      var value = session.GetString(key);
      return value == null ? default : JsonSerializer.Deserialize<T>(value);
    }
  }
#endregion
}

namespace Web.Extensions2 {
  // Alternate approach

  public static class SessionExtensions {
    public static void Set<T>(this ISession session, string key, T value) {
      session.SetString(key, JsonSerializer.Serialize(value));
    }

    public static bool TryGet<T>(this ISession session, string key, out T value) {
      var state = session.GetString(key);
      value = default;
      if (state == null)
        return false;
      value = JsonSerializer.Deserialize<T>(state);
      return true;
    }
  }
}
