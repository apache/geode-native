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
import java.io.*;

//
// NonDeserializableObject is, in general, deserializable, but on a Geode server
// it can't be deserialized because it has no default ctor, and thus can't be
// instantiated via reflection.  This is interesting because it's possible to,
// for instance, execute a function server-side which returns an instance of
// this class, which causes Geode to return a payload of type 'DataSerializable'
// with subtype 'Class', and the class name and data necessary to recreate the
// object in a client whieh supports reflection.  Since C++ doesn't have this,
// the Geode Native Client should throw an exception, and that's what we use
// this class to test.
//
public class NonDeserializableObject implements DataSerializable  {
    static {
        Instantiator.register(new NonDeserializableObjectInstantiator());
    }

   String m_str;

   public NonDeserializableObject(String str){m_str = str;}

   @Override
   public void toData(DataOutput dataOutput) throws IOException {
   }

   @Override
   public void fromData(DataInput dataInput) throws IOException, ClassNotFoundException {
   }

   public static class NonDeserializableObjectInstantiator extends Instantiator {
        public NonDeserializableObjectInstantiator() {
            super(NonDeserializableObject.class, 500);
        }

        public NonDeserializableObjectInstantiator(Class<? extends DataSerializable> c, int classId) {
            super(c, classId);
        }

        @Override
        public DataSerializable newInstance() {
            return new NonDeserializableObject("foo");
        }
    }
}

