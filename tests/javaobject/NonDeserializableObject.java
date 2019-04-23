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

