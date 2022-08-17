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
import org.apache.geode.*; // for DataSerializable
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.cache.execute.Function;
import org.apache.geode.pdx.PdxInstance;
import org.apache.geode.pdx.PdxInstanceFactory;

public class ComparePdxInstanceFunction implements Function {

  @Override
  public void execute(FunctionContext context) {
    Object arguments = context.getArguments();
    PdxInstance instance = (PdxInstance) arguments;

    char array[] = new char[70000];
    Arrays.fill(array, 'x');
    String longString = new String(array);

    PdxInstanceFactory pdxInstanceFactory = context.getCache().createPdxInstanceFactory("PdxTests.MyTestClass");
    pdxInstanceFactory.writeString("asciiField", "value");
    pdxInstanceFactory.writeString("utf8Field", "value\u20AC");
    pdxInstanceFactory.writeString("asciiHugeField", longString );
    pdxInstanceFactory.writeString("utfHugeField", longString + "\u20AC");

    PdxInstance expectedInstance = pdxInstanceFactory.create();

    boolean result = expectedInstance.equals(instance);

    context.getResultSender().lastResult(result);
  }

  private static final String ID = "ComparePdxInstanceFunction";

  @Override
  public String getId() {
    return ID;
  }

  @Override
  public boolean hasResult() {
    return true;
  }

  public void init(Properties p) {
  }

  @Override
  public boolean optimizeForWrite() {
    return true;
  }

  @Override
  public boolean isHA() {
    return false;
  }

}



