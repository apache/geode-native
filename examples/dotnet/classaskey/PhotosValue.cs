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
using System.Drawing;
using System.Collections.Generic;

namespace Apache.Geode.Examples.ClassAsKey
{
  public class PhotoMetaData : IDataSerializable
  {
    public int fullResId;
    public Bitmap thumbnailImage;

    public const int THUMB_WIDTH = 32;
    public const int THUMB_HEIGHT = 32;

    // A default constructor is required for deserialization
    public PhotoMetaData() { }

    public PhotoMetaData(int id, Bitmap thumb)
    {
      fullResId = id;
      thumbnailImage = thumb;
    }

    public void ToData(DataOutput output)
    {
      output.WriteInt32(fullResId);

      for (int i=0; i<thumbnailImage.Height; i++)
      {
        for (int j=0; j<thumbnailImage.Width; j++)
        {
          output.WriteInt32(thumbnailImage.GetPixel(i, j).ToArgb());
        }
      }
    }

    public void FromData(DataInput input)
    {
      fullResId = input.ReadInt32();

      thumbnailImage = new Bitmap(THUMB_WIDTH, THUMB_HEIGHT);
      for (int i = 0; i < thumbnailImage.Height; i++)
      {
        for (int j = 0; j < thumbnailImage.Width; j++)
        {
          thumbnailImage.SetPixel(i, j, Color.FromArgb(input.ReadInt32()));
        }
      }
    }

    public ulong ObjectSize
    {
      get { return 0; }
    }

    public static ISerializable CreateDeserializable()
    {
      return new PhotoMetaData();
    }
  }

  public class PhotosValue : IDataSerializable
  {
    public List<PhotoMetaData> photosMeta;

    // A default constructor is required for deserialization
    public PhotosValue() { }

    public PhotosValue(List<PhotoMetaData> metaData)
    {
      photosMeta = metaData;
    }

    public void ToData(DataOutput output)
    {
      output.WriteObject(photosMeta);
    }

    public void FromData(DataInput input)
    {
      photosMeta = new List<PhotoMetaData>();
      var pmd = input.ReadObject() as IList<object>;
      if (pmd != null)
      {
        foreach (var item in pmd)
        {
          photosMeta.Add((PhotoMetaData)item);
        }
      }
    }

    public ulong ObjectSize
    {
      get { return 0; }
    }

    public static ISerializable CreateDeserializable()
    {
      return new PhotosValue();
    }
  }
}


