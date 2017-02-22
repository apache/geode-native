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

#pragma once

#include "geode_defs.hpp"
#include "IPdxSerializer.hpp"
#include "PdxIdentityFieldAttribute.hpp"
using namespace System;
using namespace System::Reflection;
using namespace System::Reflection::Emit;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      /// <summary>
      /// Enumerated type for pdx FieldType
      /// </summary>
      public enum class FieldType
      {
        BOOLEAN,
				BYTE,
				CHAR,
				SHORT,
				INT,
				LONG,
				FLOAT,
				DOUBLE,
				DATE,
				STRING,
				OBJECT,
				BOOLEAN_ARRAY,
				CHAR_ARRAY,
				BYTE_ARRAY,
				SHORT_ARRAY,
				INT_ARRAY,
				LONG_ARRAY,
				FLOAT_ARRAY,
				DOUBLE_ARRAY,
				STRING_ARRAY,
				OBJECT_ARRAY,
				ARRAY_OF_BYTE_ARRAYS
      };

        ref class FieldWrapper;

		    /// <summary>
        /// This class uses .NET reflection in conjunction with
        /// <see cref="IPdxSerializer"/> to perform
        /// automatic serialization of domain objects. The implication is that the domain
        /// classes do not need to implement the <see cref="IPdxSerializable"> interface.       
        /// This implementation will serialize all relevant fields.
        /// This will not serialize the fields which has defined attribute NonSerialized.
        /// This will not serialize the static, literal and readonly fields.
        ///
        /// Use <see cref="PdxIdentityFieldAttribute"> to define member field as identity field.
        /// Identity fields are used for hashcode creation and equals methods.
        ///
        /// </summary>
        public ref class ReflectionBasedAutoSerializer : IPdxSerializer
        {
        public:

          virtual bool ToData( Object^ o,IPdxWriter^ writer );

          virtual Object^ FromData(String^ o, IPdxReader^ reader );

         /// <summary>
         /// Controls the field name that will be used in pdx for a field being auto serialized.
         /// Override this method to customize the field names that will be generated by auto serialization.
         /// It allows you to convert a local, language dependent name, to a more portable name.
         /// The returned name is the one that will show up in a <see cref="IPdxInstance" /> and that
         /// one that will need to be used to access the field when doing a query.
         /// <para>
         /// The default implementation returns the name obtained from <code>fi</code>.
         /// <para>
         /// This method is only called the first time it sees a new class. The result
         /// will be remembered and used the next time the same class is seen.
         /// </summary>
         /// <param name="fi"> the field whose name is returned.</param>
         /// <param name"type"> type the original class being serialized that owns this field.</param>
         /// <returns> the name of the field </returns>
         
          virtual String^ GetFieldName(FieldInfo^ fi, Type^ type);

         /// <summary>
         /// Controls what fields of a class that is auto serialized will be marked
         /// as pdx identity fields.
         /// Override this method to customize what fields of an auto serialized class will be
         /// identity fields.
         /// Identity fields are used when a <see cref="IPdxInstance" /> computes its hash code
         /// and checks to see if it is equal to another object.
         /// <para>
         /// The default implementation only marks fields that match an "#identity=" pattern
         /// as identity fields.
         /// <para>
         /// This method is only called the first time it sees a new class. The result
         /// will be remembered and used the next time the same class is seen.
         /// </summary>
         /// <param name="fi"> the field to test to see if it is an identity field.</param>
         /// <param name="type"> the original class being serialized that owns this field.</param>
         /// <returns> true if the field should be marked as an identity field; false if not. </returns>
         
          virtual bool IsIdentityField(FieldInfo^ fi, Type^ type);

         /// <summary>
         /// Controls what pdx field type will be used when auto serializing.
         /// Override this method to customize what pdx field type will be used
         /// for a given domain class field.
         /// <para>
         /// The default implementation uses type of field.
         /// <para>
         /// This method is only called the first time it sees a new class. The result
         /// will be remembered and used the next time the same class is seen.
         /// </summary> 
         /// <param name="fi"> the field whose pdx field type needs to be determined </param>
         /// <param name="type"> the original class being serialized that owns this field.</param>
          /// <returns> the pdx field type of the given domain class field.</returns>         
         
          virtual FieldType GetFieldType(FieldInfo^ fi, Type^ type);
  
         /// <summary>
         /// Controls what fields of a class will be auto serialized by this serializer.
         /// Override this method to customize what fields of a class will be auto serialized.
         /// The default implementation:
         /// <list type="bullet">
         /// <item>
         /// <description> excludes NonSerialized fields</description>
         /// </item>
         /// <item>
         /// <description> excludes static fields</description>
         /// </item>
         /// <item>
         /// <description> excludes literal fields</description>
         /// </item>
         /// <item>
         /// <description> excludes readonly fields </description>
         /// </item>
         /// </list>
         /// All other fields are included.
         /// This method is only called the first time it sees a new class. The result
         /// will be remembered and used the next time the same class is seen.
         /// </summary>
         /// <param name="fi"> the field being considered for serialization</param>
         /// <param name="type"> the original class being serialized that owns this field.</param>
          /// <returns> true if the field should be serialized as a pdx field; false if it should be ignored.</returns>
         
          virtual bool IsFieldIncluded(FieldInfo^ fi, Type^ type);

         /// <summary>
         /// Controls what field value is written during auto serialization.
         /// Override this method to customize the data that will be written
         /// during auto serialization.
         /// </summary>
         /// <param name="fi"> the field in question</param>
         /// <param name="type"> the original class being serialized that owns this field.</param>
         /// <param name="originalValue"> the value of the field that was read from the domain object.</param> 
          /// <returns> the actual value to write for this field. Return <code>originalValue</code>
          ///   if you decide not to transform the value. </returns>
         
          virtual Object^ WriteTransform(FieldInfo^ fi, Type^ type, Object^ originalValue);

         /// <summary>
         /// Controls what field value is read during auto deserialization.
         /// Override this method to customize the data that will be read
         /// during auto deserialization.
         /// This method will only be called if {@link #transformFieldValue}
         /// returned true.
          /// </summary>
          /// <param name="fi"> the field in question </param>
         /// <param name="type"> the original class being serialized that owns this field.
         ///   Note that this field may have been inherited from a super class by this class.</param>
         /// <param value="serializeValue"> the value of the field that was serialized for this field.</param>
          /// <returns> the actual value to write for this field. Return <code>serializedValue</code>
          ///   if you decide not to transform the value. </returns>         
          virtual Object^ ReadTransform(FieldInfo^ fi, Type^ type, Object^ serializeValue);          
        
          /// <summary>
          /// Overirde this method to create default instance of <code>className</code>
          /// Otherwise it will create instance using zer arg public constructor
          /// </summary>
          /// <param name="className"> name of the class to create default instance </param>
          /// <returns> the defaulf instance </returns>

          virtual Object^ CreateObject(String^ className);

          ReflectionBasedAutoSerializer();
        private:

          FieldType getPdxFieldType( Type^ type);

          void serializeFields(Object^ o,IPdxWriter^ writer );

          Object^ deserializeFields(String^ o, IPdxReader^ reader);
          
          bool IsPdxIdentityField(FieldInfo^ fi);

          System::Collections::Generic::Dictionary<String^, List<FieldWrapper^>^>^ classNameVsFieldInfoWrapper;
                                
          List<FieldWrapper^>^ GetFields(Type^ domaimType);

          static Type^ PdxIdentityFieldAttributeType = nullptr;
        };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

