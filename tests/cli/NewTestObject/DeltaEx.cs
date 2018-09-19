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
using Apache.Geode.Client;
namespace Apache.Geode.Client.Tests
{
  public class DeltaEx : IDelta, IDataSerializable, ICloneable
  {
    private int counter;
    private bool IsDelta;
    public static int ToDeltaCount = 0;
    public static int FromDeltaCount = 0;
    public static int ToDataCount = 0;
    public static int FromDataCount = 0;
    public static int CloneCount = 0;

    public DeltaEx()
    {
      counter = 24;
      IsDelta = false;
    }
    public DeltaEx(int count)
    {
      count = 0;
      IsDelta = false;
    }
    public bool HasDelta()
    {
      return IsDelta;
    }
    public void ToDelta(Apache.Geode.Client.DataOutput DataOut)
    {
      DataOut.WriteInt32(counter);
      ToDeltaCount++;
    }

    public void FromDelta(Apache.Geode.Client.DataInput DataIn)
    {
      int val = DataIn.ReadInt32();
      if (FromDeltaCount == 1)
      {
        FromDeltaCount++;
        throw new Apache.Geode.Client.InvalidDeltaException();
      }
      counter += val;
      FromDeltaCount++;
    }

    public void ToData(Apache.Geode.Client.DataOutput DataOut)
    {
      DataOut.WriteInt32(counter);
      ToDataCount++;
    }
    public void FromData(Apache.Geode.Client.DataInput DataIn)
    {
      counter = DataIn.ReadInt32();
      FromDataCount++;
    }

    public UInt64 ObjectSize
    {
      get
      {
        return 0;
      }
    }
    public void SetDelta(bool isDelta)
    {
      IsDelta = isDelta;
    }

    public static Apache.Geode.Client.ISerializable create()
    {
      return new DeltaEx();
    }
    public Object Clone()
    {
      CloneCount++;
      return new DeltaEx();
    }
    public int getCounter()
    {
      return counter;
    }
  }
}
