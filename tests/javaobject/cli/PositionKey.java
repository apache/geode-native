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
package javaobject.cli;

import java.util.*;
import java.io.*;
import org.apache.geode.*;
import org.apache.geode.cache.Declarable;


public class PositionKey implements DataSerializable {
  private long positionId;

  static {
     Instantiator.register(new Instantiator(javaobject.cli.PositionKey.class, 21) {
     public DataSerializable newInstance() {
        return new PositionKey();
     }
   });
  }

  /* public no-arg constructor required for DataSerializable */  
  public PositionKey() {}

  public PositionKey(long id){
    positionId = id;
  }
  
  public String toString(){
    return "PositionKey [positionId=" + positionId + "]";
  }
  
  public void fromData(DataInput in) throws IOException, ClassNotFoundException {
    this.positionId = in.readLong();
  }
  
  public void toData(DataOutput out) throws IOException {
    out.writeLong(this.positionId);
  } 
  
  public int hashCode() {
    return Objects.hash(positionId);
  }

  public boolean equals(final Object obj)
  {
      if (this == obj)
          return true;
      if (obj == null)
          return false;
      if (getClass() != obj.getClass())
          return false;
      final PositionKey other = (PositionKey) obj;

      if (positionId != other.positionId)
          return false;

      return true;
  }
}
