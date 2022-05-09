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
import org.apache.geode.cache.Declarable;

import PdxTests.DeliveryAddress;

import org.apache.geode.cache.*;
import org.apache.geode.cache.Region;
import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.cache.execute.ResultSender;

public class PutDeliveryAddress extends FunctionAdapter implements Declarable{

  public void execute(FunctionContext context) {
    Vector args = (Vector)context.getArguments();
    Iterator iter = args.iterator();

    String regionName = (String)iter.next();
    String entryName = (String)iter.next();
    String addressLine = (String)iter.next();
    String city = (String)iter.next();
    String country = (String)iter.next();
    String instructions = (String)iter.next();

    Region r = CacheFactory.getAnyInstance().getRegion(regionName);
    DeliveryAddress address = new DeliveryAddress(addressLine, city, country, instructions);

    r.put(entryName, address);
    context.getResultSender().lastResult(address.toString());
  }

  public String getId() {
    return "PutDeliveryAddress";
  }

  public void init(Properties p) {
  }

  public boolean hasResult() {
    return true;
  }
}
