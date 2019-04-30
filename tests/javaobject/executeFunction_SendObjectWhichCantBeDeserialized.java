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

import org.apache.geode.DataSerializable;
import org.apache.geode.Instantiator;
import org.apache.geode.cache.Declarable;
import org.apache.geode.cache.Region;
import org.apache.geode.cache.execute.Function;
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.cache.execute.RegionFunctionContext;

import java.util.Properties;
import java.io.*;

import javaobject.NonDeserializableObject;

public class executeFunction_SendObjectWhichCantBeDeserialized implements Function, Declarable {

    public static final String NAME = "executeFunction_SendObjectWhichCantBeDeserialized";

    @Override
    public void execute(FunctionContext context) {
        NonDeserializableObject myNonDeserializableObject = new NonDeserializableObject("123");
        context.getResultSender().lastResult(myNonDeserializableObject);
    }

    @Override
    public boolean hasResult() {
        return true;
    }

    @Override
    public boolean optimizeForWrite() {
        return true;
    }

    @Override
    public String getId() {
        return NAME;
    }

   @Override
    public boolean isHA() {
        return true;
    }

    @Override
    public void init(Properties props) {
    }
}

