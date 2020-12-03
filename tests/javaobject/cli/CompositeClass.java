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

public class CompositeClass implements Declarable, Serializable, DataSerializable {
  private TestClassA testclassA;
  private List<TestClassB> testclassBs;
  private List<TestClassC> testclassCs;

  static {
     Instantiator.register(new Instantiator(javaobject.cli.CompositeClass.class, (byte) 125) {
     public DataSerializable newInstance() {
        return new CompositeClass();
     }
   });
  }

  /* public no-arg constructor required for DataSerializable */  
  public CompositeClass() {
	
  }
  
  public TestClassA getA() {
	return testclassA;
  }

  public void setA(TestClassA a) {
	this.testclassA = a;
  }

  public List<TestClassB> getBs() {
	return testclassBs;
  }

  public void setBs(List<TestClassB> bs) {
	this.testclassBs = bs;
  }

  public List<TestClassC> getCs() {
	return testclassCs;
  }

  public void setCs(List<TestClassC> cs) {
	this.testclassCs = cs;
  }

  @Override
  public void fromData(DataInput input) throws IOException, ClassNotFoundException {
    setA((TestClassA) DataSerializer.readObject(input));
    setBs((List<TestClassB>) DataSerializer.readObject(input));
    setCs((List<TestClassC>) DataSerializer.readObject(input));
  }
  
  @Override
  public void toData(DataOutput output) throws IOException {
	try {
	  DataSerializer.writeObject(getA(), output);
	  DataSerializer.writeObject(getBs(), output);
	  DataSerializer.writeObject(getCs(), output);
	}
	catch (Exception e) {
	  e.printStackTrace();
	  throw new IOException(e);
	}
  } 

  @Override
  public boolean equals(Object other) {
    if (other==null) return false;
    if (!(other instanceof CompositeClass)) return false;
    
    CompositeClass testclass = (CompositeClass) other;
    
    if (this.testclassA != testclass.testclassA) return false;
    if (this.testclassBs != testclass.testclassBs) return false;
    if (this.testclassCs != testclass.testclassCs) return false;

    return true;    
  }
  
  public int hashCode() {
     final int prime = 31;
     int result = 1;
     result = prime * result + testclassA.hashCode();
     result = prime * result + testclassBs.hashCode();
     result = prime * result + testclassCs.hashCode();
    
    return result;
  }
}
