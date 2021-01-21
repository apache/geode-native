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

public class TestClassB implements Declarable, Serializable, DataSerializable {
  private int Width;
  private int Height;
  private String Name;

  static {
     Instantiator.register(new Instantiator(javaobject.cli.TestClassB.class, (byte) 101) {
     public DataSerializable newInstance() {
        return new TestClassB();
     }
   });
  }

  /* public no-arg constructor required for DataSerializable */  
  public TestClassB() {}

  public TestClassB(int width, int height, String name){
    Width = width;
    Height = height;
    Name = name;
  }
  
  public String toString(){
    return "TestClassB [Width="+Width+" Name="+Name+ " Height="+Height+"]";
  }
  
  public void fromData(DataInput in) throws IOException, ClassNotFoundException {
    this.Width = in.readInt();
    this.Height = in.readInt();
    this.Name = in.readUTF();
  }
  
  public void toData(DataOutput out) throws IOException {
    out.writeInt(this.Width);
    out.writeInt(this.Height);
    out.writeUTF(this.Name);
  } 
  
  public static boolean compareForEquals(Object first, Object second) {
    if (first == null && second == null) return true;
    if (first != null && first.equals(second)) return true;
    return false;
  }
  
  public boolean equals(Object other) {
    if (other==null) return false;
    if (!(other instanceof TestClassB)) return false;
    
    TestClassB testclass = (TestClassB) other;
    
    if (this.Width != testclass.Width) return false;
    if (this.Name != testclass.Name) return false;
    if (this.Height != testclass.Height) return false;

    return true;    
  }
  
  public int hashCode() {
    Integer width = new Integer(Width);
    Integer height = new Integer(Height);
    String name = new String(Name);

    int hashcode =
	  width.hashCode() ^
	  name.hashCode() ^
	  height.hashCode();
    
    return hashcode;
  }
}
