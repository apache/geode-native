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
using Apache.Geode.Client;
using System;
using System.Collections.Generic;

namespace Apache.Geode.Examples.ClassAsKey
{
  public class PhotosKey : IDataSerializable, ICacheableKey
  {
    public List<CacheableString> people;
    public CacheableDate rangeStart;
    public CacheableDate rangeEnd;

    // A default constructor is required for deserialization
    public PhotosKey() { }

    public PhotosKey(List<CacheableString> names, CacheableDate start, CacheableDate end)
    {
      people = names;
      rangeStart = start;
      rangeEnd = end;
    }

    public override string ToString()
    {
      string result = "{";
      for (int i = 0; i < people.Count; i++)
      {
        result += people[i];
        if (i<people.Count-1)
          result += ", ";
      }
      result += "} from ";
      return result + rangeStart.ToString() + " to " +
        rangeEnd.ToString();
    }

    public void ToData(DataOutput output)
    {
      output.WriteObject(people);
      output.WriteObject(rangeStart);
      output.WriteObject(rangeEnd);
    }

    public void FromData(DataInput input)
    {
      people = (List<CacheableString>)input.ReadObject();
      rangeStart = (CacheableDate)input.ReadObject();
      rangeEnd = (CacheableDate)input.ReadObject();
    }

    public ulong ObjectSize
    {
      get { return 0; }
    }

    public bool Equals(ICacheableKey other)
    {
      return Equals((object)other);
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

      PhotosKey otherKey = (PhotosKey)obj;
      return (people == otherKey.people &&
        rangeStart == otherKey.rangeStart &&
        rangeEnd == otherKey.rangeEnd);
    }

    public override int GetHashCode()
    {
      int prime = 31;
      int result = 1;
      foreach (CacheableString cs in people)
      {
        result = result * prime + cs.GetHashCode();
      }

      result = result * prime + rangeStart.GetHashCode();
      result = result * prime + rangeEnd.GetHashCode();

      return result;
    }

    public static ISerializable CreateDeserializable()
    {
      return new PhotosKey();
    }
  }
}


