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

public class TestClassC implements Declarable, Serializable, DataSerializable {
  private String Color;
  private String Make;
  private int Year;

  static {
     Instantiator.register(new Instantiator(javaobject.cli.TestClassC.class, (byte) 102) {
     public DataSerializable newInstance() {
        return new TestClassC();
     }
   });
  }

  /* public no-arg constructor required for DataSerializable */  
  public TestClassC() {}

  public TestClassC(String color, String make, int year){
    Color = color;
    Make = make;
    Year = year;
  }
  
  public String toString(){
    return "TestClassC [Color="+Color+" Make="+Make+ " Year="+Year +"]";
  }
  
  public void fromData(DataInput in) throws IOException, ClassNotFoundException {
    this.Color = in.readUTF();
    this.Make = in.readUTF();
    this.Year = in.readInt();
  }
  
  public void toData(DataOutput out) throws IOException {
    out.writeUTF(this.Color);
    out.writeUTF(this.Make);
    out.writeInt(this.Year);

  } 
  
  public static boolean compareForEquals(Object first, Object second) {
    if (first == null && second == null) return true;
    if (first != null && first.equals(second)) return true;
    return false;
  }
  
  public boolean equals(Object other) {
    if (other==null) return false;
    if (!(other instanceof TestClassC)) return false;
    
    TestClassC testclass = (TestClassC) other;
    
    if (this.Color != testclass.Color) return false;
    if (this.Make != testclass.Make) return false;
    if (this.Year != testclass.Year) return false;

    return true;    
  }
  
  public int hashCode() {
    String color = new String(Color);
    String make = new String(Make);
    Integer year = new Integer(Year);

    int hashcode =
	  color.hashCode() ^
	  make.hashCode() ^
	  year.hashCode();
    
    return hashcode;
  }
}
