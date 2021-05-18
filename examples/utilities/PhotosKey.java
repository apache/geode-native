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
package javaobject;

import java.util.*;
import java.io.*;
import org.apache.geode.*;
import org.apache.geode.cache.Declarable;
import org.apache.geode.DataSerializer;

import org.apache.geode.logging.internal.log4j.api.LogService;

public class PhotosKey implements DataSerializable {
  public List<String> people;
  public Date rangeStart;
  public Date rangeEnd;

  static {
     Instantiator.register(new Instantiator(javaobject.PhotosKey.class, 500) {
     public DataSerializable newInstance() {
        return new PhotosKey();
     }
   });
  }

  /* public no-arg constructor required for DataSerializable */  
  public PhotosKey() {}

  public PhotosKey(List<String> names, Date start, Date end){
    people = names;
	rangeStart = start;
	rangeEnd = end;
  }
  
  public void fromData(DataInput in) throws IOException, ClassNotFoundException {
    this.people = DataSerializer.readObject(in);
	this.rangeStart = DataSerializer.readDate(in);
	this.rangeEnd = DataSerializer.readDate(in);
  }
  
  public void toData(DataOutput out) throws IOException {
    DataSerializer.writeObject(this.people, out);
    DataSerializer.writeDate(this.rangeStart, out);
    DataSerializer.writeDate(this.rangeEnd, out);
  } 

  public int hashCode() {
	LogService.getLogger().warn("hashCode = {}", Objects.hash(people, rangeStart, rangeEnd));
	return Objects.hash(people, rangeStart, rangeEnd);
  }

  public boolean equals(final Object obj)
  {
    if (this == obj)
      return true;
    if (obj == null)
      return false;
    if (getClass() != obj.getClass())
      return false;
    final PhotosKey other = (PhotosKey) obj;

    if (!people.equals(other.people))
      return false;

	if (!rangeStart.equals(other.rangeStart) || !rangeEnd.equals(other.rangeEnd))
	  return false;

    return true;
  }
}
