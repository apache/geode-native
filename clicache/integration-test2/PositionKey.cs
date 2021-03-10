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

namespace Apache.Geode.Client.IntegrationTests
{
  using Apache.Geode.Client;
  public class PositionKey : ICacheableKey, IDataSerializable
  {
    private long m_positionId;

	public PositionKey() { }
	public PositionKey(long positionId)
	{
		m_positionId = positionId;
	}
	public long PositionId
	{
		get
		{
			return m_positionId;
		}
		set
		{
			m_positionId = value;
		}
	}

	public void FromData(DataInput input)
	{
		m_positionId = input.ReadInt64();
	}

	public void ToData(DataOutput output)
    {
      output.WriteInt64(m_positionId);
    }

    public UInt64 ObjectSize
    {
      get
      {
		return 8;
	  }
    }

	public static IDataSerializable CreateDeserializable()
	{
		return new PositionKey();
	}

	public static int GetClassID()
	{
		return 77;
	}

	public bool Equals(ICacheableKey other)
	{
	  return Equals((Object)other);
	}

	public override bool Equals(object obj)
	{
		if (this == obj)
		{
			return true;
		}

		if (GetType() != obj.GetType())
		{
			return false;
		}

		PositionKey otherKey = (PositionKey)obj;
		return (m_positionId == otherKey.m_positionId);
	}

	public override int GetHashCode()
	{
	  return Objects.Hash(m_positionId);
	}
  }
}
