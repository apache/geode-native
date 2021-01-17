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


public class PhotosKey implements DataSerializable {
  public List<String> people;
  //public Date rangeStart;
  //public Date rangeEnd;
  public int rangeStart;
  public int rangeEnd;

  static {
     Instantiator.register(new Instantiator(javaobject.PhotosKey.class, 500) {
     public DataSerializable newInstance() {
        return new PhotosKey();
     }
   });
  }

  /* public no-arg constructor required for DataSerializable */  
  public PhotosKey() {}

  //public PhotosKey(List<String> names, Date start, Date end){
  public PhotosKey(List<String> names, int start, int end){
    people = names;
	rangeStart = start;
	rangeEnd = end;
  }
  
  public void fromData(DataInput in) throws IOException, ClassNotFoundException {
    this.people = DataSerializer.readObject(in);
	//this.rangeStart = (Date)DataSerializer.readDate(in);
	//this.rangeEnd = (Date)DataSerializer.readDate(in);
	this.rangeStart = in.readInt();
	this.rangeEnd = in.readInt();
  }
  
  public void toData(DataOutput out) throws IOException {
    DataSerializer.writeObject(this.people, out);
    //DataSerializer.writeDate(this.rangeStart, out);
    //DataSerializer.writeDate(this.rangeEnd, out);
    out.writeInt(this.rangeStart);
    out.writeInt(this.rangeEnd);
  } 
  

  public int hashCode() {
	final int prime = 31;
	int result = 1;
	for( String s : people )
	{
		result = result * prime + s.hashCode();
	}

	//result = result * prime + rangeStart.hashCode();
	//result = result * prime + rangeEnd.hashCode();
	result = result * prime + rangeStart;
	result = result * prime + rangeEnd;
	return result;
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
	//for (int i=0; i<people.size(); i++)
	//{
	//  if (people.get(i).equals(other.people.get(i)))
	//	return false;
	//}

	if (rangeStart != other.rangeStart || rangeEnd != other.rangeEnd)
	  return false;

    return true;
  }
}
