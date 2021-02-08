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
using System.Drawing;
using Apache.Geode.Client;

namespace Apache.Geode.Examples.ClassAsKey
{
  public class Program
  {
    static Cache cache;
    static Random rand;

    public static void Main(string[] args)
    {
      const int MAXPHOTOKEYS = 10;
      const int MAXPHOTOSPERKEY = 5;

      IRegion<PhotosKey, PhotosValue> photosMetaData = CreateRegion();

      Console.WriteLine("Populating the photos region\n");

      PhotosKey[] keys = new PhotosKey[MAXPHOTOKEYS];
      PhotosValue[] values = new PhotosValue[MAXPHOTOKEYS];

      DateTime start;
      DateTime end;

      rand = new Random();
      int numPhotos;
      int photoId = 0;

      for (int i=0; i<MAXPHOTOKEYS; i++)
      {
        ChooseDateRange(out start, out end);
        keys[i] = new PhotosKey(ChoosePeople(), start, end);

        numPhotos = rand.Next(0, MAXPHOTOSPERKEY+1);
        List<PhotoMetaData> metaData = new List<PhotoMetaData>();
        for (int j=0; j<numPhotos; j++)
        {
          PhotoMetaData meta = new PhotoMetaData();
          meta.fullResId = photoId++;
          meta.thumbnailImage = ChooseThumb();
          metaData.Add(meta);
        }
        values[i] = new PhotosValue(metaData);

        Console.WriteLine("Inserting " + numPhotos + " photos for key: " + keys[i].ToString() +
          " with hashCode = " + Objects.Hash(keys[i].people, keys[i].rangeStart, keys[i].rangeEnd));

        photosMetaData.Put(keys[i], values[i]);
      }

      // Verify the region was populated properly
      photoId = 0;
      Console.WriteLine();
      for (int k = 0; k < MAXPHOTOKEYS; k++)
      {
        Console.WriteLine("Fetching photos for key: " + keys[k].ToString());

        PhotosValue value = photosMetaData.Get(keys[k]);
        PhotoMetaData meta;
        for (int p = 0; p < value.photosMeta.Count; p++)
        {
          Console.WriteLine("   Fetching photo number " + p);

          meta = value.photosMeta[p];
          if (meta.fullResId != photoId)
            Console.WriteLine("      ERROR: Expected fullResId = " + photoId + " but actual = " + meta.fullResId);

          bool thumbValid = true;
          for (int i=0; i<PhotoMetaData.THUMB_HEIGHT; i++)
          {
            for (int j=0; j<PhotoMetaData.THUMB_WIDTH; j++)
            {
              if (meta.thumbnailImage.GetPixel(i,j) != values[k].photosMeta[p].thumbnailImage.GetPixel(i,j))
              {
                Console.WriteLine("      ERROR: Unexpected thumb for photoId = " + photoId);
                thumbValid = false;
                break;
              }
            }

            if (!thumbValid)
              break;
          }

          photoId++;
        }
      }

      cache.Close();
      Console.ReadLine();
    }

    public static IRegion<PhotosKey, PhotosValue> CreateRegion()
    {
      cache = new CacheFactory()
          .Set("log-level", "debug")
          .Set("log-file", "c:/temp/classaskey.log")
          .Create();

      Console.WriteLine("Registering for data serialization");

        cache.TypeRegistry.RegisterType(PhotosKey.CreateDeserializable, 500);
        cache.TypeRegistry.RegisterType(PhotosValue.CreateDeserializable, 501);
        cache.TypeRegistry.RegisterType(PhotoMetaData.CreateDeserializable, 502);

        cache.GetPoolManager()
            .CreateFactory()
            .AddLocator("localhost", 10334)
            .Create("pool");

      var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
          .SetPoolName("pool");
      IRegion<PhotosKey, PhotosValue> photosMetaData = regionFactory.Create<PhotosKey, PhotosValue>("photosMetaData");
      return photosMetaData;
    }

    public static List<String> ChoosePeople()
    {
      List<String> availablePeople = new List<String> {
        "Alice",
        "Bob",
        "Carol",
        "Ted"
      };

      List<String> chosenPeople = new List<String>();

      // Choose at least one person
      int numChosen = rand.Next(1, availablePeople.Count+1);

      int index;
      int numAvailable = availablePeople.Count;

      for (int i=0; i<numChosen; i++)
      {
        // Choose someone not already chosen
        index = rand.Next(numAvailable);
        chosenPeople.Add(availablePeople[index]);

        // Update available people
        availablePeople.RemoveAt(index);
        numAvailable--;
      }

      // Sort the chosen. We only care who is chosen, not the order they're chosen.
      chosenPeople.Sort();
      return chosenPeople;
    }

    public static void ChooseDateRange(out DateTime start, out DateTime end)
    {
      //Choose start and end dates between Jan 1, 1970 and now
      var earliestStart = new DateTime(1970, 1, 1);
      int numAvailableDays = (int)(DateTime.Now - earliestStart).TotalDays;

      var startIndex = rand.Next(numAvailableDays);
      start = earliestStart.AddDays(startIndex);

      int numRemainingDays = (int)(DateTime.Now - start).TotalDays;
      end = start.AddDays(rand.Next(numRemainingDays));
    }

    public static Bitmap ChooseThumb()
    {
      int thumbWidth = 32;
      int thumbHeight = 32;

      Bitmap thumb = new Bitmap(thumbWidth, thumbHeight);
      for (int j=0; j<thumbHeight; j++)
      {
        for (int i = 0; i < thumbWidth; i++)
        {
          thumb.SetPixel(i, j, Color.FromArgb(rand.Next(256), rand.Next(256), rand.Next(256)));
        }
      }

      return thumb;
    }
  }
}


