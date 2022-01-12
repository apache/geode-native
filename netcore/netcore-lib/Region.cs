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
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

using Apache.Geode.Client;


//[DllImport(Constants.libPath, CharSet = CharSet.Auto)]
//static extern bool apache_geode_Region_PutString(IntPtr region, IntPtr key,
//                                                         IntPtr value);

//[DllImport(Constants.libPath, CharSet = CharSet.Auto)]
//static extern bool apache_geode_Region_PutByteArray(IntPtr region, IntPtr key,
//                                                            IntPtr value, int length);

//[DllImport(Constants.libPath, CharSet = CharSet.Auto)]
//static extern bool apache_geode_Region_PutByteArrayForInt32Key(IntPtr region,
//      [param: MarshalAs(UnmanagedType.I4)] Int32 key,
//      IntPtr value, int length);

//[DllImport(Constants.libPath, CharSet = CharSet.Auto)]
//static extern IntPtr apache_geode_Region_GetString(IntPtr region, IntPtr key);



namespace Apache.Geode.Client {

  //public class Region : GeodeNativeObject {
  public class Region<TKey, TValue> : GeodeNativeObject, IRegion<TKey, TValue>
  {
    internal Region(IntPtr regionFactory, string regionName) {
      var regionNamePtr = Marshal.StringToCoTaskMemUTF8(regionName);
      _containedObject = CBindings.apache_geode_RegionFactory_CreateRegion(regionFactory, regionNamePtr);
      Marshal.FreeCoTaskMem(regionNamePtr);
    }

    public void Put(TKey key, TValue value)
    {
      var keyType = key.GetType();
      if (!keyType.IsSerializable) {
        throw new Exception("Error: key is not Serializable.");
      }

      var valueType = value.GetType();
      if (!valueType.IsSerializable) {
        throw new Exception("Error: value is not Serializable.");
      }

      if (key is string ||
          key is System.Byte ||
          key is System.Int16 ||
          key is System.Int32) {
      }
      else {
        throw new NotImplementedException();
      }

      if (value is string ||
          value is System.Byte ||
          value is System.Int16 ||
          value is System.Int32) {
      }
      else {
        throw new NotImplementedException();
      }

      int keyLength = 0;
      int valueLength = 0;
      IntPtr valuePtr = (IntPtr)0;
      IntPtr keyPtr = (IntPtr)0;
      Constants.DSCode keyCode = Constants.DotNetToDSCode[Type.GetTypeCode(keyType)];
      Constants.DSCode valueCode = Constants.DotNetToDSCode[Type.GetTypeCode(valueType)];

      // Serialize the key

      if (keyType.IsPrimitive) {
        keyLength = CacheablePrimitive<TKey>.SerializeNoSwap(key, ref keyPtr);
      }
      else if (key is string) {
        keyPtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(key));
      }

      // Serialize the value

      if (valueType.IsPrimitive) {
        valueLength = CacheablePrimitive<TValue>.Serialize(value, ref valuePtr);
      }
      else if (value is string) {
        valuePtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(value));
      }

      if (keyType.IsPrimitive && valueType.IsPrimitive) {
        CBindings.apache_geode_Region_Put(_containedObject,
            keyPtr, keyLength + sizeof(Constants.DSCode),
            valuePtr, valueLength + sizeof(Constants.DSCode));
      }
      else if (valueType.IsPrimitive) {
        CBindings.apache_geode_Region_PutByteArray(_containedObject, keyPtr,
          valuePtr, valueLength + sizeof(Constants.DSCode));
      }
      else {
        CBindings.apache_geode_Region_PutString(_containedObject, keyPtr, valuePtr);
      }

      Marshal.FreeCoTaskMem(keyPtr);
      Marshal.FreeCoTaskMem(valuePtr);
    }


    public static class CacheablePrimitive<TPrim> {
      public static int Serialize(TPrim value, ref IntPtr valuePtr) {
        int length = 0;
        Type type = value.GetType();
        switch (Type.GetTypeCode(value.GetType()))
        {
          case TypeCode.Byte:
            length = WriteObject(Convert.ToByte(value), ref valuePtr);
            break;
          case TypeCode.Int16:
            length = WriteObject(Convert.ToInt16(value), ref valuePtr);
            break;
          case TypeCode.Int32:
            length = WriteObject(Convert.ToInt32(value), ref valuePtr);
            break;
        }
        return length;
      }

      public static int SerializeNoSwap(TPrim value, ref IntPtr valuePtr)
      {
        int length = 0;
        Type type = value.GetType();
        switch (Type.GetTypeCode(value.GetType()))
        {
          case TypeCode.Byte:
            length = WriteObject(Convert.ToByte(value), ref valuePtr);
            break;
          case TypeCode.Int16:
            length = WriteObjectNoSwap(Convert.ToInt16(value), ref valuePtr);
            break;
          case TypeCode.Int32:
            length = WriteObjectNoSwap(Convert.ToInt32(value), ref valuePtr);
            break;
        }
        return length;
      }

      public static TValue Deserialize(IntPtr valPtr, int size)
      //public static TValue Deserialize(byte[] byteArray)

      {
        Byte[] byteArray = new Byte[size];
        Marshal.Copy(valPtr, byteArray, 0, size);
        Marshal.FreeCoTaskMem(valPtr);
        Constants.DSCode dsCode = (Constants.DSCode)byteArray[0];
        int dsCodeSize = sizeof(Constants.DSCode);
        TValue rVal;
        switch (dsCode)
        {
          case Constants.DSCode.CacheableASCIIString:
            string str = Encoding.ASCII.GetString(byteArray, dsCodeSize, byteArray.Length - dsCodeSize);
            rVal = (TValue)Convert.ChangeType(str, typeof(TValue));
            break;
          case Constants.DSCode.CacheableByte:
            Byte int8 = byteArray[dsCodeSize];
            rVal = (TValue)Convert.ChangeType(int8, typeof(TValue));
            break;
          case Constants.DSCode.CacheableInt16:
            Int16 int16 =
              (Int16)((byteArray[dsCodeSize] << 8) |
              (byteArray[dsCodeSize+1]));
            rVal = (TValue)Convert.ChangeType(int16, typeof(TValue));
            break;
          case Constants.DSCode.CacheableInt32:
            Int32 int32 =
              (byteArray[dsCodeSize] << 24) |
              (byteArray[dsCodeSize+1] << 16) |
              (byteArray[dsCodeSize+2] << 8) |
              (byteArray[dsCodeSize+3]);
            rVal = (TValue)Convert.ChangeType(int32, typeof(TValue));
            break;
          default:
            return default(TValue);
        }
        return rVal;
      }

      public static int WriteObject(Byte value, ref IntPtr valuePtr)
      {
        int valueLength = sizeof(Byte);
        valuePtr = Marshal.AllocCoTaskMem(valueLength + sizeof(Constants.DSCode));

        Marshal.StructureToPtr<Int32>((Int32)Constants.DSCode.CacheableByte, valuePtr, false);

        Marshal.StructureToPtr<byte>((byte)(value), valuePtr + sizeof(Constants.DSCode), false);

        return valueLength;
      }

      public static int WriteObject(Int16 value, ref IntPtr valuePtr)
      {
        int valueLength = sizeof(Int16);
        valuePtr = Marshal.AllocCoTaskMem(valueLength + sizeof(Constants.DSCode));

        Marshal.StructureToPtr<Int32>((Int32)Constants.DSCode.CacheableInt16, valuePtr, false);

        Marshal.StructureToPtr<byte>((byte)(value >> 8), valuePtr + sizeof(Constants.DSCode), false);
        Marshal.StructureToPtr<byte>((byte)(value), valuePtr + sizeof(Constants.DSCode)+1, false);

        return valueLength;
      }

      public static int WriteObjectNoSwap(Int16 value, ref IntPtr valuePtr)
      {
        int valueLength = sizeof(Int16);
        valuePtr = Marshal.AllocCoTaskMem(valueLength + sizeof(Constants.DSCode));

        Marshal.StructureToPtr<Int32>((Int32)Constants.DSCode.CacheableInt16, valuePtr, false);

        Marshal.StructureToPtr<Int16>(value, valuePtr + sizeof(Constants.DSCode), false);

        return valueLength;
      }

      public static int WriteObject(Int32 value, ref IntPtr valuePtr)
      {
        int valueLength = sizeof(Int32);
        valuePtr = Marshal.AllocCoTaskMem(valueLength + sizeof(Constants.DSCode));

        Marshal.StructureToPtr<Int32>((Int32)Constants.DSCode.CacheableInt32, valuePtr, false);

        Marshal.StructureToPtr<byte>((byte)(value >> 24), valuePtr+sizeof(Constants.DSCode), false);
        Marshal.StructureToPtr<byte>((byte)(value >> 16), valuePtr+sizeof(Constants.DSCode)+1, false);
        Marshal.StructureToPtr<byte>((byte)(value >> 8),  valuePtr+sizeof(Constants.DSCode)+2, false);
        Marshal.StructureToPtr<byte>((byte)(value),       valuePtr+sizeof(Constants.DSCode)+3, false);

        return valueLength;
      }

      public static int WriteObjectNoSwap(Int32 value, ref IntPtr valuePtr)
      {
        int valueLength = sizeof(Int32);
        valuePtr = Marshal.AllocCoTaskMem(valueLength + sizeof(Constants.DSCode));

        Marshal.StructureToPtr<Int32>((Int32)Constants.DSCode.CacheableInt32, valuePtr, false);

        Marshal.StructureToPtr<Int32>(value, valuePtr + sizeof(Constants.DSCode), false);
                                                         
        return valueLength;                              
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

    public TValue Get(TKey key)
    {
      var keyType = key.GetType();
      if (!keyType.IsSerializable)
      {
        throw new Exception("Error: key is not Serializable.");
      }

      if (key is string ||
          key is System.Int16 ||
          key is System.Int32)
      {
      }
      else
      {
        throw new NotImplementedException();
      }

      int keyLength = 0;
      IntPtr valuePtr = (IntPtr)0;
      IntPtr keyPtr = (IntPtr)0;

      if (keyType.IsPrimitive)
      {
        keyLength = CacheablePrimitive<TKey>.Serialize(key, ref keyPtr);
      }
      else if (key is string)
      {
        keyPtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(key));
      }

      var valPtr = (IntPtr)0;
      int valueLength = 0;

      if (keyType.IsPrimitive) {
        CBindings.apache_geode_Region_Get(_containedObject, keyPtr, keyLength, ref valPtr, ref valueLength);
        return CacheablePrimitive<TValue>.Deserialize(valPtr, valueLength);
      }
      else
      {
        CBindings.apache_geode_Region_GetByteArray(_containedObject, keyPtr, ref valPtr, ref valueLength);
        return CacheablePrimitive<TValue>.Deserialize(valPtr, valueLength);
      }
    }

    //public string GetString(string key) {
    //  var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
    //  var result =
    //      Marshal.PtrToStringUTF8(CBindings.apache_geode_Region_GetString(_containedObject, keyPtr));
    //  Marshal.FreeCoTaskMem(keyPtr);
    //  return result;
    //}

    //public byte[] GetByteArray(TKey key) {
    //  var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
    //  var valPtr = (IntPtr)0;
    //  int size = 0;
    //  CBindings.apache_geode_Region_GetByteArray(_containedObject, keyPtr, ref valPtr, ref size);
    //  if (size > 0) {
    //    Byte[] byteArray = new Byte[size];
    //    Marshal.Copy(valPtr, byteArray, 0, size);
    //    Marshal.FreeCoTaskMem(valPtr);
    //    return byteArray;
    //  } else
    //    return null;
    //}

    //public byte[] GetByteArray(int key)
    //{
    //  var valPtr = (IntPtr)0;
    //  int size = 0;
    //  apache_geode_Region_GetByteArrayForInt32Key(_containedObject, key, ref valPtr, ref size);
    //  if (size > 0)
    //  {
    //    Byte[] byteArray = new Byte[size];
    //    Marshal.Copy(valPtr, byteArray, 0, size);
    //    Marshal.FreeCoTaskMem(valPtr);
    //    return byteArray;
    //  }
    //  else
    //    return null;
    //}

    public bool Remove(string key) {
      var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
      CBindings.apache_geode_Region_Remove(_containedObject, keyPtr);
      Marshal.FreeCoTaskMem(keyPtr);
      return true; // TODO: Needs to ensure removal
    }

    public bool ContainsValueForKey(string key) {
      var keyPtr = Marshal.StringToCoTaskMemUTF8(key);
      bool result = CBindings.apache_geode_Region_ContainsValueForKey(_containedObject, keyPtr);
      Marshal.FreeCoTaskMem(keyPtr);
      return result;
    }

    protected override void DestroyContainedObject() {
      CBindings.apache_geode_DestroyRegion(_containedObject);
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
