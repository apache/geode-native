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

public class TestClassA implements Declarable, Serializable, DataSerializable {
  private int Id;
  private String Name;
  private short NumSides;

  static {
     Instantiator.register(new Instantiator(javaobject.cli.TestClassA.class, (byte) 100) {
     public DataSerializable newInstance() {
        return new TestClassA();
     }
   });
  }

  /* public no-arg constructor required for DataSerializable */  
  public TestClassA() {}

  public TestClassA(int id, String name, short numSides){
    Id = id;
    Name = name;;
    NumSides = numSides;
  }
  
  public String toString(){
    return "TestClassA [Id="+Id+" Name="+Name+ " NumSides="+NumSides +"]";
  }
  
  public void fromData(DataInput in) throws IOException, ClassNotFoundException {
    this.Id = in.readInt();
    this.Name = in.readUTF();
    this.NumSides = in.readShort();
  }
  
  public void toData(DataOutput out) throws IOException {
    out.writeInt(this.Id);
    out.writeUTF(this.Name);
    out.writeShort(this.NumSides);
  } 
  
  public static boolean compareForEquals(Object first, Object second) {
    if (first == null && second == null) return true;
    if (first != null && first.equals(second)) return true;
    return false;
  }
  
  public boolean equals(Object other) {
    if (other==null) return false;
    if (!(other instanceof Position)) return false;
    
    TestClassA testclass = (TestClassA) other;
    
    if (this.Id != testclass.Id) return false;
    if (this.Name != testclass.Name) return false;
    if (this.NumSides != testclass.NumSides) return false;

    return true;    
  }
  
  public int hashCode() {
    Integer i = new Integer(Id);
    String name = new String(Name);
    Short numSides = new Short(NumSides);

    int hashcode =
	  i.hashCode() ^
	  name.hashCode() ^
	  numSides.hashCode();
    
    return hashcode;
  }
}
