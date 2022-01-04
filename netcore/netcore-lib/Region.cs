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
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

namespace Apache.Geode.Client {
  public class Region : GeodeNativeObject {
    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern IntPtr apache_geode_RegionFactory_CreateRegion(IntPtr cache,
                                                                         IntPtr regionName);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern bool apache_geode_Region_PutString(IntPtr region, IntPtr key,
                                                             IntPtr value);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern bool apache_geode_Region_PutByteArray(IntPtr region, IntPtr key,
                                                                IntPtr value, int length);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern bool apache_geode_Region_PutByteArrayForInt32Key(IntPtr region,
          [param: MarshalAs(UnmanagedType.I4)] Int32 key,
          IntPtr value, int length);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern IntPtr apache_geode_Region_GetString(IntPtr region, IntPtr key);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern void apache_geode_Region_GetByteArray(IntPtr region, IntPtr key,
                                                                ref IntPtr value, ref int size);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern void apache_geode_Region_GetByteArrayForInt32Key(IntPtr region,
          [param: MarshalAs(UnmanagedType.I4)] Int32 key,
          ref IntPtr value, ref int size);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern void apache_geode_Region_Remove(IntPtr region, IntPtr key);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern bool apache_geode_Region_ContainsValueForKey(IntPtr region,
                                                                       IntPtr key);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    private static extern IntPtr apache_geode_DestroyRegion(IntPtr region);

    internal Region(IntPtr regionFactory, string regionName) {
      var regionNamePtr = Marshal.StringToCoTaskMemUTF8(regionName);
      _containedObject = apache_geode_RegionFactory_CreateRegion(regionFactory, regionNamePtr);
      Marshal.FreeCoTaskMem(regionNamePtr);
    }

    public void Put<TValue>(string key, TValue value)
    {
      var keyPtr = Marshal.StringToCoTaskMemUTF8(key);

      // Ensure serializable
      var valueType = value.GetType();
      if (!valueType.IsSerializable)
        throw new Exception("Error: value is not Serializable.");

      TypeCode valueTypeCode = Type.GetTypeCode(typeof(TValue));
      Constants.DSCode dsCode = (Constants.DSCode)Constants.DotNetToDSCode[valueTypeCode];

      Int32 int32 = 0;
      Int16 int16 = 0;
      int valueLength;
      IntPtr valuePtr = (IntPtr)0;

      switch (dsCode)
      {
        case Constants.DSCode.CacheableString:
          var strPtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(value));
          apache_geode_Region_PutString(_containedObject, keyPtr, strPtr);
          Marshal.FreeCoTaskMem(keyPtr);
          Marshal.FreeCoTaskMem(strPtr);
          break;
        case Constants.DSCode.CacheableInt32:
          valueLength = 4;
          valuePtr = Marshal.AllocCoTaskMem(valueLength + 1);
          Marshal.StructureToPtr<byte>((byte)dsCode, valuePtr, false);
          int32 = Convert.ToInt32(value);
          Marshal.StructureToPtr<byte>((byte)(int32 >> 24), valuePtr+1, false);
          Marshal.StructureToPtr<byte>((byte)(int32 >> 16), valuePtr+2, false);
          Marshal.StructureToPtr<byte>((byte)(int32 >> 8),  valuePtr+3, false);
          Marshal.StructureToPtr<byte>((byte)(int32),       valuePtr+4, false);
          apache_geode_Region_PutByteArray(_containedObject, keyPtr, valuePtr, valueLength + 1);
          Marshal.FreeCoTaskMem(keyPtr);
          Marshal.FreeCoTaskMem(valuePtr);
          break;
        case Constants.DSCode.CacheableInt16:
          valueLength = 2;
          valuePtr = Marshal.AllocCoTaskMem(valueLength + 1);
          Marshal.StructureToPtr<byte>((byte)dsCode, valuePtr, false);
          int16 = Convert.ToInt16(value);
          Marshal.StructureToPtr<byte>((byte)(int16 >> 8), valuePtr + 1, false);
          Marshal.StructureToPtr<byte>((byte)(int16),      valuePtr + 2, false);
          apache_geode_Region_PutByteArray(_containedObject, keyPtr, valuePtr, valueLength + 1);
          Marshal.FreeCoTaskMem(keyPtr);
          Marshal.FreeCoTaskMem(valuePtr);
          break;
      }
    }

    //public void PutByteArray(Int32 key, byte[] value)
    //{
    //  //var keyPtr = Marshal.AllocCoTaskMem(4);
    //  var valuePtr = Marshal.AllocCoTaskMem(value.Length);
    //  Marshal.Copy(value, 0, valuePtr, value.Length);
    //  apache_geode_Region_PutByteArrayForInt32Key(_containedObject, key, valuePtr, value.Length);
    //  Marshal.FreeCoTaskMem(valuePtr);
    //}

    //byte[] ObjectToByteArray(object obj)
    //{
    //  if (obj == null)
    //    return null;
    //  BinaryFormatter bf = new BinaryFormatter();
    //  using (MemoryStream ms = new MemoryStream())
    //  {
    //    bf.Serialize(ms, obj); { }
    //    return ms.ToArray();
    //  }
    //}

    //Span<byte> ConvertToByteArray<T>(ref T value) where T : unmanaged {
    //  //unsafe {
    //  //int length = sizeof(T);
    //  //Span<byte> bytes = value;

    //  //  Buffer.BlockCopy(Array src, int srcOffset, Array dst, int dstOffset, int length);
    //  //}
    //  return value;
    //}

    //private T Deserialize<T>(byte[] param)
    //{
    //  using (MemoryStream ms = new MemoryStream(param))
    //  {
    //    IFormatter br = new BinaryFormatter();
    //    return (T)br.Deserialize(ms);
    //  }
    //}

    //public void Put<TKey, TValue>(ref TKey key, ref TValue value)
    //  where TKey : unmanaged
    //  where TValue : unmanaged
    //{
    //  // Ensure serializable
    //  var keyType = key.GetType();
    //  if (!keyType.IsSerializable)
    //    throw new Exception("Error: key is not Serializable.");
    //  var valueType = value.GetType();
    //  if (!valueType.IsSerializable)
    //    throw new Exception("Error: value is not Serializable.");

    //  TypeCode keyTypeCode = Type.GetTypeCode(typeof(TKey));
    //  TypeCode valueTypeCode = Type.GetTypeCode(typeof(TValue));

    //  Constants.DSCode keyDSCode = Constants.DotNetToDSCode[keyTypeCode];
    //  Constants.DSCode valueDSCode = Constants.DotNetToDSCode[valueTypeCode];

    //  if (keyDSCode == Constants.DSCode.CacheableString) {
    //    var keyPtr = Marshal.StringToCoTaskMemUTF8(key as string);
    //    var valuePtr = Marshal.StringToCoTaskMemUTF8(value);
    //    apache_geode_Region_PutString(_containedObject, keyPtr, valuePtr);
    //    Marshal.FreeCoTaskMem(keyPtr);
    //    Marshal.FreeCoTaskMem(valuePtr);
    //  }

    //}
    //fixed (TKey* keyPtr = &key, TValue* valPtr = &value)
    //  {
    //    apache_geode_Region_PutByteArray(IntPtr region,
    //                                     IntPtr keyPtr, sizeof(TKey),
    //                                     IntPtr valPtr, sizeof(TValue));
    //  }

    //}
    //  // Can't use sizeof with generic params (CS0233), so pick scratch size
    //  // we know will be big enough.
    //  //Span<byte> stackSpan = stackalloc byte[100];
    //  //Span<TKey> keySpan = 
    //  //nativeSpan[0] = (byte)keyTypeCode;
    //  //nativeSpan.Copy<T>(ReadOnlySpan<T> source, Span<T> destination);
    //  //unsafe
    //  //{
    //  //  Span<TKey> slicedKey = nativeSpan.Slice(start: 1, length: sizeof(TKey));

    //  //  //Buffer.BlockCopy(((byte[])((object)key)), 0, dst: (byte[])((object)nativeSpan), dstOffset: 1, count: sizeof(TKey));
    //  //  for (int i = 0; i < sizeof(TKey); i++)
    //  //    nativeSpan[i+1] = &key;
    //  //}

    //  //for (int ctr = 0; ctr < nativeSpan.Length; ctr++)
    //  //  nativeSpan[ctr] = data++;


    //  //val.TryFormat(valBytes, numBytes);
    //  //byte[] serializedBytes = val.Ser

    //  if (value.GetType() == typeof(int) || value.GetType() == typeof(uint)) {
    //    var byteArray = new byte[5];
    //    byteArray[0] = 57;
    //    int v = Convert.ToInt32(value);
    //    byteArray[1] = (byte)((v >> 24));
    //    byteArray[2] = (byte)((v >> 16));
    //    byteArray[3] = (byte)((v >> 8));
    //    byteArray[4] = (byte)((v));
    //    PutByteArray(key.ToString(), byteArray);
    //  }
    //}

    public TValue Get<TKey, TValue>(TKey key)
    {
      // This is a generic, so can't do any marshaling directly.
      if (key.GetType() == typeof(string))
      {
        var byteArray = GetByteArray(key.ToString());
        Constants.DSCode dsCode = (Constants.DSCode)byteArray[0];
        switch (dsCode) {
          case Constants.DSCode.CacheableASCIIString:
            string str = Encoding.ASCII.GetString(byteArray, 1, byteArray.Length-1);
            return (TValue)Convert.ChangeType(str, typeof(TValue));
          case Constants.DSCode.CacheableInt32:
            Int32 int32 =
              (byteArray[1] << 24) |
              (byteArray[2] << 16) |
              (byteArray[3] << 8) |
              (byteArray[4]);
            return (TValue)Convert.ChangeType(int32, typeof(TValue));
          case Constants.DSCode.CacheableInt16:
            Int16 int16 =
              (Int16)((byteArray[1] << 8) |
              (byteArray[2]));
            return (TValue)Convert.ChangeType(int16, typeof(TValue));

          default:
            return default(TValue);
        }

      }
      else
        return default(TValue);
    }

    public string GetString(string key) {
      var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
      var result =
          Marshal.PtrToStringUTF8(apache_geode_Region_GetString(_containedObject, keyPtr));
      Marshal.FreeCoTaskMem(keyPtr);
      return result;
    }

    public byte[] GetByteArray(string key) {
      var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
      var valPtr = (IntPtr)0;
      int size = 0;
      apache_geode_Region_GetByteArray(_containedObject, keyPtr, ref valPtr, ref size);
      if (size > 0) {
        Byte[] byteArray = new Byte[size];
        Marshal.Copy(valPtr, byteArray, 0, size);
        Marshal.FreeCoTaskMem(valPtr);
        return byteArray;
      } else
        return null;
    }

    public byte[] GetByteArray(int key)
    {
      var valPtr = (IntPtr)0;
      int size = 0;
      apache_geode_Region_GetByteArrayForInt32Key(_containedObject, key, ref valPtr, ref size);
      if (size > 0)
      {
        Byte[] byteArray = new Byte[size];
        Marshal.Copy(valPtr, byteArray, 0, size);
        Marshal.FreeCoTaskMem(valPtr);
        return byteArray;
      }
      else
        return null;
    }

    public void Remove(string key) {
      var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
      apache_geode_Region_Remove(_containedObject, keyPtr);
      Marshal.FreeCoTaskMem(keyPtr);
    }

    public bool ContainsValueForKey(string key) {
      var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
      bool result = apache_geode_Region_ContainsValueForKey(_containedObject, keyPtr);
      Marshal.FreeCoTaskMem(keyPtr);
      return result;
    }

    protected override void DestroyContainedObject() {
      apache_geode_DestroyRegion(_containedObject);
      _containedObject = IntPtr.Zero;
    }
  }

  //  public class GenericRegion<TVal> : Region
  //  {
  //    // private Region regionBase_;
  //    internal GenericRegion<TVal>(IntPtr regionFactory, string regionName) {}
  //  //{
  //  //  //regionBase_ = new Region(regionFactory, regionName);
  //  //  var regionNamePtr = Marshal.StringToCoTaskMemUTF8(regionName);
  //  //_containedObject = apache_geode_RegionFactory_CreateRegion(regionFactory, regionNamePtr);
  //  //Marshal.FreeCoTaskMem(regionNamePtr);
  //  //}

  //}

  //public class Region<TVal>
  //{
  //  Region region_;
  //  internal Region(RegionFactory regionFactory, string regionName)
  //  {
  //    region_ = regionFactory.CreateRegion(regionName);
  //  }

  //  public TVal Get<TKey>(TKey key)
  //  {
  //    // This is a generic, so can't do any marshaling directly.
  //    //if (key.GetType() == typeof(string))
  //    {
  //      var value = region_.GetByteArray(key.ToString());
  //      return (TVal)Convert.ChangeType(value, typeof(TVal));
  //      //return BitConverter.ToInt32(value);
  //    }
  //  }
  //}
}
