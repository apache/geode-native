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

namespace Apache.Geode.Client {

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

      if (value is string) {
      }
      else {
        throw new NotImplementedException();
      }

      int keyLength = 0;
      int valueLength = 0;
      IntPtr valuePtr = (IntPtr)0;
      IntPtr keyPtr = (IntPtr)0;

      if (keyType.IsPrimitive) {
        keyLength = CacheablePrimitive<TKey>.Serialize(key, ref keyPtr);
      }
      else if (key is string) {
        keyPtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(key));
      }

      if (valueType.IsPrimitive) {
        valueLength = CacheablePrimitive<TValue>.Serialize(value, ref valuePtr);
      }
      else if (value is string) {
        valuePtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(value));
      }

      if (keyType.IsPrimitive) {
        CBindings.apache_geode_Region_PutByteArray(_containedObject,
            keyPtr, keyLength + sizeof(Constants.DSCode),
            valuePtr);
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

      public static TValue Deserialize(IntPtr valPtr, int size)
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

        Marshal.StructureToPtr<byte>((byte)Constants.DSCode.CacheableByte, valuePtr, false);

        Marshal.StructureToPtr<byte>((byte)(value), valuePtr + sizeof(Constants.DSCode), false);

        return valueLength;
      }

      public static int WriteObject(Int16 value, ref IntPtr valuePtr)
      {
        int valueLength = sizeof(Int16);
        valuePtr = Marshal.AllocCoTaskMem(valueLength + sizeof(Constants.DSCode));

        Marshal.StructureToPtr<byte>((byte)Constants.DSCode.CacheableInt16, valuePtr, false);

        Marshal.StructureToPtr<byte>((byte)(value >> 8), valuePtr + sizeof(Constants.DSCode), false);
        Marshal.StructureToPtr<byte>((byte)(value), valuePtr + sizeof(Constants.DSCode)+1, false);

        return valueLength;
      }


      public static int WriteObject(Int32 value, ref IntPtr valuePtr)
      {
        int valueLength = sizeof(Int32);
        valuePtr = Marshal.AllocCoTaskMem(valueLength + sizeof(Constants.DSCode));

        Marshal.StructureToPtr<byte>((byte)Constants.DSCode.CacheableInt32, valuePtr, false);

        Marshal.StructureToPtr<byte>((byte)(value >> 24), valuePtr+sizeof(Constants.DSCode), false);
        Marshal.StructureToPtr<byte>((byte)(value >> 16), valuePtr+sizeof(Constants.DSCode)+1, false);
        Marshal.StructureToPtr<byte>((byte)(value >> 8),  valuePtr+sizeof(Constants.DSCode)+2, false);
        Marshal.StructureToPtr<byte>((byte)(value),       valuePtr+sizeof(Constants.DSCode)+3, false);

        return valueLength;
      }
                                                

    }

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
        CBindings.apache_geode_Region_Get(_containedObject, keyPtr,
          keyLength+sizeof(Constants.DSCode), ref valPtr, ref valueLength);
        return CacheablePrimitive<TValue>.Deserialize(valPtr, valueLength);
      }
      else
      {
        CBindings.apache_geode_Region_GetByteArray(_containedObject, keyPtr, ref valPtr, ref valueLength);
        return CacheablePrimitive<TValue>.Deserialize(valPtr, valueLength);
      }
    }

    public bool Remove(TKey key) {
      var keyType = key.GetType();
      IntPtr keyPtr = (IntPtr)0;
      int keyLength = 0;
      if (keyType.IsPrimitive)
      {
        keyLength = CacheablePrimitive<TKey>.Serialize(key, ref keyPtr);
      }
      else if (key is string)
      {
        keyPtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(key));
      }
      CBindings.apache_geode_Region_Remove(_containedObject, keyPtr);
      Marshal.FreeCoTaskMem(keyPtr);
      return true; // TODO: Needs to ensure removal
    }

    public bool ContainsValueForKey(TKey key) {
      var keyType = key.GetType();
      IntPtr keyPtr = (IntPtr)0;
      int keyLength = 0;
      if (keyType.IsPrimitive)
      {
        keyLength = CacheablePrimitive<TKey>.Serialize(key, ref keyPtr);
      }
      else if (key is string)
      {
        keyPtr = Marshal.StringToCoTaskMemUTF8(Convert.ToString(key));
      }
      bool result = CBindings.apache_geode_Region_ContainsValueForKey(_containedObject, keyPtr);
      Marshal.FreeCoTaskMem(keyPtr);
      return result;
    }

    protected override void DestroyContainedObject() {
      CBindings.apache_geode_DestroyRegion(_containedObject);
      _containedObject = IntPtr.Zero;
    }
  }
}
