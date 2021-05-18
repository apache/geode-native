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

#pragma once

using namespace System;

namespace Apache {
namespace Geode {

/// <summary>
/// Provides hash code functions similar to those used by Geode server in 
/// Java's java.util.Objects and java.util.Arrays classes.
///
/// Example:
/// <pre>
/// class CustomKey
/// {
///   private int a;
///   private double b;
///   private String c;
///
///   public CustomKey(int a, double b, String c)
///   {
///     this.a = a;
///     this.b = b;
///     this.c = c;
///   }
///
///   override public int GetHashCode()
///   {
///     return Objects.Hash(a, b, c);
///   }
/// };
/// </pre>
/// </summary>
 public ref class Objects {
 public:
  /// <summary>
  /// Hashes consistent with java.util.Objects.hash(Object ...).
  /// </summary>
  /// <param name="values">
  /// Variable arguments to combine into hash.
  /// </param>
  static Int32 Hash(... array<Object^>^ values);

  
  /// <summary>
  /// Hashes consistent with java.util.Objects.hashCode(Object).
  /// </summary>
  /// <param name="value">
  /// Object to hash.
  /// </param>
  static Int32 GetHashCode(Object^ value);


  /// <summary>
  /// Hashes consistent with java.lang.String.hashCode().
  /// </summary>
  /// <param name="value">
  /// String to hash.
  /// </param>
  static Int32 GetHashCode(String^ value);

  /// <summary>
  /// Hashes consistent with java.lang.Character.hashCode().
  /// </summary>
  /// <param name="value">
  /// Character to hash.
  /// </param>
  static Int32 GetHashCode(Char value);

  /// <summary>
  /// Hashes consistent with java.lang.Boolean.hashCode().
  /// </summary>
  /// <param name="value">
  /// Boolean to hash.
  /// </param>
  static Int32 GetHashCode(Boolean value);

  /// <summary>
  /// Hashes consistent with java.lang.Byte.hashCode().
  /// </summary>
  /// <param name="value">
  /// Byte to hash.
  /// </param>
  static Int32 GetHashCode(SByte value);

  /// <summary>
  /// Hashes consistent with java.lang.Short.hashCode().
  /// </summary>
  /// <param name="value">
  /// Short to hash.
  /// </param>
  static Int32 GetHashCode(Int16 value);

  /// <summary>
  /// Hashes consistent with java.lang.Integer.hashCode().
  /// </summary>
  /// <param name="value">
  /// Integer to hash.
  /// </param>
  static Int32 GetHashCode(Int32 value);

  /// <summary>
  /// Hashes consistent with java.lang.Long.hashCode().
  /// </summary>
  /// <param name="value">
  /// Long to hash.
  /// </param>
  static Int32 GetHashCode(Int64 value);

  /// <summary>
  /// Hashes consistent with java.lang.Float.hashCode().
  /// </summary>
  /// <param name="value">
  /// FLoat to hash.
  /// </param>
  static Int32 GetHashCode(Single value);

  /// <summary>
  /// Hashes consistent with java.lang.Double.hashCode().
  /// </summary>
  /// <param name="value">
  /// Double to hash.
  /// </param>
  static Int32 GetHashCode(Double value);

  /// <summary>
  /// Hashes consistent with java.util.Date.hashCode().
  /// </summary>
  /// <param name="value">
  /// Date to hash.
  /// </param>
  static Int32 GetHashCode(DateTime^ value);

  /// <summary>
  /// Hashes consistent with java.lang.Date.hashCode().
  /// </summary>
  /// <param name="value">
  /// Date to hash.
  /// </param>
  static Int32 GetHashCode(DateTime value);

  /// <summary>
  /// Hashes consistent with java.util.Arrays.hashCode(Object[]) or
  /// java.util.List.hashCode().
  /// </summary>
  /// <param name="value">
  /// Array or List like collection to hash.
  /// </param>
  static Int32 GetHashCode(System::Collections::ICollection^ collection);

  /// <summary>
  /// Hashes consistent with java.util.Map.hashCode().
  /// </summary>
  /// <param name="dictionary">
  /// Map to hash.
  /// </param>
  static Int32 GetHashCode(System::Collections::IDictionary^ dictionary);
};

}  // namespace Geode
}  // namespace Apache
