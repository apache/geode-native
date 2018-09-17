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

namespace Apache.Geode.Client.UnitTests
{
  using Apache.Geode.Client;

  [Serializable]
  class CustomSerializableObject
  {
    public String key;
    public String value;

    public CustomSerializableObject()
    {
      key = "key";
      value = "value";
    }
  }

  class DefaultType : IDataSerializable
  {
    bool m_cacheableBoolean;
    int m_cacheableInt32;
    int[] m_cacheableInt32Array = null;
    string m_cacheableFileName = null;
    string m_CacheableStringASCII = null;
    string[] m_cacheableStringArray = null;
    //CacheableHashSet m_cacheableHashSet = null;
    Dictionary<Object, Object> m_cacheableHashMap;
    //DateTime m_cacheableDate = null;
    IList<object> m_cacheableVector = null;
    object m_cacheableObject = null;

    bool m_initialized = false;

    public DefaultType()
    { 
    
    }

    public DefaultType(bool initialized)
    {
      if (initialized)
      {
        Log.Fine("DefaultType in constructor");
        m_initialized = true;
        
        m_cacheableBoolean = true;
        
        m_cacheableInt32 = 1000;
        
        m_cacheableInt32Array =new int[]{1,2,3};
        
        m_cacheableFileName = "geode.txt";
        
        m_CacheableStringASCII = "asciistring";
        
        m_cacheableStringArray = new string[] { "one", "two" };
             
        m_cacheableHashMap = new Dictionary<Object, Object>();
        m_cacheableHashMap.Add("key-hm", "value-hm");

        m_cacheableVector = new List<object>();
        m_cacheableVector.Add("one-vec");
        m_cacheableVector.Add("two-vec");
      } 
    }

    public bool CBool
    {
      get { return m_cacheableBoolean; }
    }

    public int CInt
    {
      get { return m_cacheableInt32; }
    }

    public int[] CIntArray
    {
      get { return m_cacheableInt32Array; }
    }

    public string CFileName
    {
      get { return m_cacheableFileName; }
    }

    public string CString
    {
      get { return m_CacheableStringASCII; }
    }

    public string[] CStringArray
    {
      get { return m_cacheableStringArray; }
    }

    public IDictionary<object, object> CHashMap
    {
      get { return m_cacheableHashMap; }
    }

    public IList<object> CVector
    {
      get { return m_cacheableVector; }
    }

    public object CObject
    {
      get { return m_cacheableObject; }
    }

    #region IDataSerializable Members

    public void FromData(DataInput input)
    {

      m_cacheableBoolean = input.ReadBoolean();
      m_cacheableInt32 = input.ReadInt32();
      int arraylen = input.ReadArrayLen();
      m_cacheableInt32Array = new int[arraylen];
      for (int item = 0; item < arraylen; item++)
      {
        m_cacheableInt32Array[item] = input.ReadInt32();
      }
      m_cacheableFileName = input.ReadUTF();
      m_CacheableStringASCII = input.ReadUTF();
      arraylen = input.ReadArrayLen();
      m_cacheableStringArray = new string[arraylen];
      for (int item = 0; item < arraylen; item++)
      {
        m_cacheableStringArray[item] = input.ReadUTF();
      }
      m_cacheableHashMap = new Dictionary<Object, Object>();
      input.ReadDictionary((System.Collections.IDictionary)m_cacheableHashMap);
      arraylen = input.ReadArrayLen();
      m_cacheableVector = new object[arraylen];
      for (int item = 0; item < arraylen; item++)
      {
        m_cacheableVector[item] = input.ReadObject();
      }
    }

    public UInt64 ObjectSize
    {
      get { return 100; }//need to implement
    }

    public void ToData(DataOutput output)
    {
      if (m_initialized)
      {
        output.WriteBoolean(m_cacheableBoolean);
        output.WriteInt32(m_cacheableInt32);
        output.WriteArrayLen(m_cacheableInt32Array.Length);
        foreach (int item in m_cacheableInt32Array)
        {
          output.WriteInt32(item);
        }
        output.WriteUTF(m_cacheableFileName);
        output.WriteUTF(m_CacheableStringASCII);
        output.WriteArrayLen(m_cacheableStringArray.Length);
        foreach (string item in m_cacheableStringArray)
        {
          output.WriteUTF(item);
        }
        output.WriteDictionary((System.Collections.IDictionary)m_cacheableHashMap);
        output.WriteArrayLen(m_cacheableVector.Count);
        foreach (object item in m_cacheableVector)
        {
          output.WriteObject(item);
        }
      }
    }

    #endregion

    public static ISerializable CreateDeserializable()
    {
      return new DefaultType();
    }
  }
}
