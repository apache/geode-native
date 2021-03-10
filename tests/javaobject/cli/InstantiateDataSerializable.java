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
import org.apache.geode.*; // for DataSerializable
import org.apache.geode.cache.Declarable;

import org.apache.geode.cache.Region;
import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.cache.execute.ResultSender;
import org.apache.geode.cache.execute.RegionFunctionContext;
import org.apache.geode.cache.partition.PartitionRegionHelper;

public class InstantiateDataSerializable extends FunctionAdapter implements Declarable{

public void execute(FunctionContext context) {

  Instantiator.register(new Instantiator(javaobject.cli.PositionKey.class, 21) {
    public DataSerializable newInstance() {
      return new javaobject.cli.PositionKey();
    }
  });

  Instantiator.register(new Instantiator(javaobject.cli.Position.class, 22) {
    public DataSerializable newInstance() {
      return new javaobject.cli.Position();
    }
  });

  Instantiator.register(new Instantiator(javaobject.cli.TestClassA.class, 100) {
    public DataSerializable newInstance() {
      return new javaobject.cli.TestClassA();
    }
  });

  Instantiator.register(new Instantiator(javaobject.cli.TestClassB.class, 101) {
    public DataSerializable newInstance() {
      return new javaobject.cli.TestClassB();
    }
  });

  Instantiator.register(new Instantiator(javaobject.cli.TestClassC.class, 102) {
    public DataSerializable newInstance() {
      return new javaobject.cli.TestClassC();
    }
  });

  Instantiator.register(new Instantiator(javaobject.cli.CompositeClass.class, 125) {
    public DataSerializable newInstance() {
      return new javaobject.cli.CompositeClass();
    }
  });


  ResultSender sender = context.getResultSender();
    sender.lastResult(0);
  } 

  public String getId() {
    return "InstantiateDataSerializable";
  }

  public void init(Properties arg0) {
  }
}
