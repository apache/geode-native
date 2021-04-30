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

#include "../begin_native.hpp"
#include <geode/Cache.hpp>
#include <CacheRegionHelper.hpp>
#include <CacheImpl.hpp>
#include <CachePerfStats.hpp>
#include <Utils.hpp>
#include "../end_native.hpp"

#include "PdxInstanceImpl.hpp"
#include "PdxHelper.hpp"
#include "PdxTypeRegistry.hpp"
#include "PdxType.hpp"
#include "PdxLocalWriter.hpp"
#include "../DataInput.hpp"
#include "DotNetTypes.hpp"
#include "PdxType.hpp"
#include "../Cache.hpp"

#include "../GeodeClassIds.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace Internal
      {
        using namespace System;
        using namespace System::Text;

        //this is for PdxInstanceFactory
        PdxInstanceImpl::PdxInstanceImpl(Dictionary<String^, Object^>^ fieldVsValue, PdxType^ pdxType, Apache::Geode::Client::Cache^ cache)
        {
          m_updatedFields = fieldVsValue;
          m_typeId = 0;
          m_own = false;
          m_buffer = NULL;
          m_bufferLength = 0;
          m_pdxType = pdxType;
          m_cache = cache;
          m_cachePerfStats = &CacheRegionHelper::getCacheImpl(cache->GetNative().get())->getCachePerfStats();
          m_pdxType->InitializeType(cache);//to generate static position map

          //need to initialize stream. this will call todata and in toData we will have stream
          auto output = m_cache->GetNative()->createDataOutput();

          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          Apache::Geode::Client::Internal::PdxHelper::SerializePdx(%mg_output, this);
        }

        String^ PdxInstanceImpl::GetClassName()
        {
          if (m_typeId != 0)
          {
            PdxType^ pdxtype = m_cache->GetPdxTypeRegistry()->GetPdxType(m_typeId);
            if (pdxtype == nullptr)//will it ever happen
              throw gcnew IllegalStateException("PdxType is not defined for PdxInstance: " + m_typeId);
            return pdxtype->PdxClassName;
          }
          //will it ever happen
          throw gcnew IllegalStateException("PdxInstance typeid is not defined yet, to get classname.");
        }

        Object^ PdxInstanceImpl::GetObject()
        {
          DataInput^ dataInput = gcnew DataInput(m_buffer, m_bufferLength, m_cache);
          dataInput->setRootObjectPdx(true);
          System::Int64 sampleStartNanos = Utils::startStatOpTime();
          Object^ ret = Internal::PdxHelper::DeserializePdx(dataInput, true, m_typeId, m_bufferLength,
            CacheRegionHelper::getCacheImpl(m_cache->GetNative().get())->getSerializationRegistry().get());

          if(m_cachePerfStats)
          {
            Utils::updateStatOpTime(m_cachePerfStats->getStat(),
                                    m_cachePerfStats->getPdxInstanceDeserializationTimeId(),
                                    sampleStartNanos);
            m_cachePerfStats->incPdxInstanceDeserializations();
          }
          return ret;
        }

        bool PdxInstanceImpl::HasField(String^ fieldName)
        {
          PdxType^ pt = getPdxType();
          return pt->GetPdxField(fieldName) != nullptr;
        }

        IList<String^>^ PdxInstanceImpl::GetFieldNames()
        {
          PdxType^ pt = getPdxType();

          IList<PdxFieldType^>^ pdxFieldList = pt->PdxFieldList;
          IList<String^>^ retList = gcnew List<String^>();

          for (int i = 0; i < pdxFieldList->Count; i++)
          {
            PdxFieldType^ currPf = pdxFieldList[i];
            retList->Add(currPf->FieldName);
          }

          return retList;
        }

        bool PdxInstanceImpl::IsIdentityField(String^ fieldName)
        {
          PdxType^ pt = getPdxType();
          PdxFieldType^ pft = pt->GetPdxField(fieldName);

          return pft != nullptr && pft->IdentityField;
        }

        Object^ PdxInstanceImpl::GetField(String^ fieldName)
        {
          PdxType^ pt = getPdxType();

          PdxFieldType^ pft = pt->GetPdxField(fieldName);

          if (pft == nullptr)
          {
            // throw gcnew IllegalStateException("PdxInstance doesn't has field " + fieldName);    
            return nullptr;
          }

          {
            DataInput^ dataInput = gcnew DataInput(m_buffer, m_bufferLength, m_cache);
            dataInput->setPdxdeserialization(true);

            int pos = getOffset(dataInput, pt, pft->SequenceId);
            //Log::Debug("PdxInstanceImpl::GetField object pos " + (pos + 8) );
            dataInput->ResetAndAdvanceCursorPdx(pos);

            Object^ tmp = this->readField(dataInput, fieldName, pft->TypeId);

            //dataInput->ResetPdx(0);

            return tmp;
          }
          return nullptr;
        }

        void PdxInstanceImpl::setOffsetForObject(DataInput^ dataInput, PdxType^ pt, int sequenceId)
        {
          int pos = getOffset(dataInput, pt, sequenceId);
          dataInput->ResetAndAdvanceCursorPdx(pos);
        }

        int PdxInstanceImpl::getOffset(DataInput^ dataInput, PdxType^ pt, int sequenceId)
        {
          dataInput->ResetPdx(0);

          int offsetSize = 0;
          int serializedLength = 0;
          int pdxSerializedLength = dataInput->GetPdxBytes();
          if (pdxSerializedLength <= 0xff)
            offsetSize = 1;
          else if (pdxSerializedLength <= 0xffff)
            offsetSize = 2;
          else
            offsetSize = 4;

          if (pt->NumberOfVarLenFields > 0)
            serializedLength = pdxSerializedLength - ((pt->NumberOfVarLenFields - 1) * offsetSize);
          else
            serializedLength = pdxSerializedLength;

          System::Byte* offsetsBuffer = dataInput->GetCursor() + serializedLength;

          return pt->GetFieldPosition(sequenceId, offsetsBuffer, offsetSize, serializedLength);
        }

        int PdxInstanceImpl::getSerializedLength(DataInput^ dataInput, PdxType^ pt)
        {
          dataInput->ResetPdx(0);

          int offsetSize = 0;
          int serializedLength = 0;
          int pdxSerializedLength = dataInput->GetPdxBytes();
          if (pdxSerializedLength <= 0xff)
            offsetSize = 1;
          else if (pdxSerializedLength <= 0xffff)
            offsetSize = 2;
          else
            offsetSize = 4;

          if (pt->NumberOfVarLenFields > 0)
            serializedLength = pdxSerializedLength - ((pt->NumberOfVarLenFields - 1) * offsetSize);
          else
            serializedLength = pdxSerializedLength;

          return serializedLength;
        }

        bool PdxInstanceImpl::Equals(Object^ other)
        {
          if (other == nullptr)
            return false;

          PdxInstanceImpl^ otherPdx = dynamic_cast<PdxInstanceImpl^>(other);

          if (otherPdx == nullptr)
            return false;

          PdxType^ myPdxType = getPdxType();
          PdxType^ otherPdxType = otherPdx->getPdxType();

          if (!otherPdxType->PdxClassName->Equals(myPdxType->PdxClassName))
            return false;

          int hashCode = 1;

          //PdxType^ pt = getPdxType();

          IList<PdxFieldType^>^ myPdxIdentityFieldList = getIdentityPdxFields(myPdxType);
          IList<PdxFieldType^>^ otherPdxIdentityFieldList = otherPdx->getIdentityPdxFields(otherPdxType);

          equatePdxFields(myPdxIdentityFieldList, otherPdxIdentityFieldList);
          equatePdxFields(otherPdxIdentityFieldList, myPdxIdentityFieldList);

          DataInput^ myDataInput = gcnew DataInput(m_buffer, m_bufferLength, m_cache);
          myDataInput->setPdxdeserialization(true);
          DataInput^ otherDataInput = gcnew DataInput(otherPdx->m_buffer, otherPdx->m_bufferLength, m_cache);
          otherDataInput->setPdxdeserialization(true);

          bool isEqual = false;
          int fieldTypeId = -1;
          for (int i = 0; i < myPdxIdentityFieldList->Count; i++)
          {
            PdxFieldType^ myPFT = myPdxIdentityFieldList[i];
            PdxFieldType^ otherPFT = otherPdxIdentityFieldList[i];

            // Log::Debug("pdxfield " + ((myPFT != Default_PdxFieldType)? myPFT->FieldName: otherPFT->FieldName));
            if (myPFT == Default_PdxFieldType)
            {
              fieldTypeId = otherPFT->TypeId;
              /*Object^ val = otherPdx->GetField(otherPFT->FieldName);
              if(val == nullptr || (int)val == 0 || (bool)val == false)
              continue;*/
            }
            else if (otherPFT == Default_PdxFieldType)
            {
              fieldTypeId = myPFT->TypeId;
              /*Object^ val = this->GetField(myPFT->FieldName);
              if(val == nullptr || (int)val == 0 || (bool)val == false)
              continue;*/
            }
            else
            {
              fieldTypeId = myPFT->TypeId;
            }

            switch (fieldTypeId)
            {
            case PdxFieldTypes::CHAR:
            case PdxFieldTypes::BOOLEAN:
            case PdxFieldTypes::BYTE:
            case PdxFieldTypes::SHORT:
            case PdxFieldTypes::INT:
            case PdxFieldTypes::LONG:
            case PdxFieldTypes::DATE:
            case PdxFieldTypes::FLOAT:
            case PdxFieldTypes::DOUBLE:
            case PdxFieldTypes::STRING:
            case PdxFieldTypes::BOOLEAN_ARRAY:
            case PdxFieldTypes::CHAR_ARRAY:
            case PdxFieldTypes::BYTE_ARRAY:
            case PdxFieldTypes::SHORT_ARRAY:
            case PdxFieldTypes::INT_ARRAY:
            case PdxFieldTypes::LONG_ARRAY:
            case PdxFieldTypes::FLOAT_ARRAY:
            case PdxFieldTypes::DOUBLE_ARRAY:
            case PdxFieldTypes::STRING_ARRAY:
            case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
            {
              if (!compareRawBytes(otherPdx, myPdxType, myPFT, myDataInput, otherPdxType, otherPFT, otherDataInput))
                return false;
              break;
            }
            case PdxFieldTypes::OBJECT:
            {
              Object^ object = nullptr;
              Object^ otherObject = nullptr;
              if (myPFT != Default_PdxFieldType)
              {
                setOffsetForObject(myDataInput, myPdxType, myPFT->SequenceId);
                object = myDataInput->ReadObject();
              }

              if (otherPFT != Default_PdxFieldType)
              {
                otherPdx->setOffsetForObject(otherDataInput, otherPdxType, otherPFT->SequenceId);
                otherObject = otherDataInput->ReadObject();
              }


              if (object != nullptr)
              {
                if (object->GetType()->IsArray)
                {
                  if (object->GetType()->GetElementType()->IsPrimitive)//primitive type
                  {
                    if (!compareRawBytes(otherPdx, myPdxType, myPFT, myDataInput, otherPdxType, otherPFT, otherDataInput))
                      return false;
                  }
                  else//array of objects
                  {
                    if (!deepArrayEquals(object, otherObject))
                      return false;
                  }
                }
                else//object but can be hashtable, list etc
                {
                  if (!deepArrayEquals(object, otherObject))
                    return false;
                }
              }
              else if (otherObject != nullptr)
              {
                return false;
                //hashCode = 31 * hashCode; // this may be issue 
              }

              break;
            }
            case PdxFieldTypes::OBJECT_ARRAY:
            {
              Object^ objectArray = nullptr;
              Object^ otherObjectArray = nullptr;

              if (myPFT != Default_PdxFieldType)
              {
                setOffsetForObject(myDataInput, myPdxType, myPFT->SequenceId);
                objectArray = myDataInput->ReadObjectArray();
              }

              if (otherPFT != Default_PdxFieldType)
              {
                otherPdx->setOffsetForObject(otherDataInput, otherPdxType, otherPFT->SequenceId);
                otherObjectArray = otherDataInput->ReadObjectArray();
              }

              if (!deepArrayEquals(objectArray, otherObjectArray))
                return false;
              break;
            }
            default:
            {
              throw gcnew IllegalStateException("PdxInstance not found typeid " + myPFT->TypeId);
            }
            }

          }
          return true;
        }

        bool PdxInstanceImpl::compareRawBytes(PdxInstanceImpl^ other, PdxType^ myPT, PdxFieldType^ myF, DataInput^ myDataInput, PdxType^ otherPT, PdxFieldType^ otherF, DataInput^ otherDataInput)
        {
          if (myF != Default_PdxFieldType && otherF != Default_PdxFieldType)
          {
            int pos = getOffset(myDataInput, myPT, myF->SequenceId);
            int nextpos = getNextFieldPosition(myDataInput, myF->SequenceId + 1, myPT);
            myDataInput->ResetAndAdvanceCursorPdx(pos);

            int otherPos = other->getOffset(otherDataInput, otherPT, otherF->SequenceId);
            int otherNextpos = other->getNextFieldPosition(otherDataInput, otherF->SequenceId + 1, otherPT);
            otherDataInput->ResetAndAdvanceCursorPdx(otherPos);

            if ((nextpos - pos) != (otherNextpos - otherPos))
              return false;

            for (int i = pos; i < nextpos; i++)
            {
              if (myDataInput->ReadSByte() != otherDataInput->ReadSByte())
                return false;
            }
            //Log::Debug("compareRawBytes returns true" );
            return true;
          }
          else
          {
            DataInput^ tmpDI = nullptr;
            if (myF == Default_PdxFieldType)
            {
              int otherPos = other->getOffset(otherDataInput, otherPT, otherF->SequenceId);
              int otherNextpos = other->getNextFieldPosition(otherDataInput, otherF->SequenceId + 1, otherPT);
              return hasDefaultBytes(otherF, otherDataInput, otherPos, otherNextpos);
            }
            else
            {
              int pos = getOffset(myDataInput, myPT, myF->SequenceId);
              int nextpos = getNextFieldPosition(myDataInput, myF->SequenceId + 1, myPT);
              return hasDefaultBytes(myF, myDataInput, pos, nextpos);
            }
          }
        }

        void PdxInstanceImpl::equatePdxFields(IList<PdxFieldType^>^ my, IList<PdxFieldType^>^ other)
        {
          //Log::Debug("PdxInstanceImpl::equatePdxFields");

          for (int i = 0; i < my->Count; i++)
          {
            PdxFieldType^ myF = my[i];
            if (myF != Default_PdxFieldType)
            {
              Log::Debug("field name " + myF->ToString());
              int otherIdx = other->IndexOf(myF);

              if (otherIdx == -1)//field not there
              {
                if (i < other->Count)
                {
                  PdxFieldType^ tmp = other[i];
                  other[i] = Default_PdxFieldType;
                  other->Add(tmp);
                }
                else
                {
                  other->Add(Default_PdxFieldType);
                }
              }
              else if (otherIdx != i)
              {
                PdxFieldType^ tmp = other[i];
                other[i] = other[otherIdx];
                other[otherIdx] = tmp;
              }
            }
          }

          //if(my->Count != other->Count)
          //{
          //  for(int i = 0; i < other->Count; i++)
          //  {
          //    PdxFieldType^ otherF = other[i];
          //    int myIdx = my->IndexOf(otherF);
          //    if(myIdx == -1)//this is the field not there
          //    {
          //      my[i] = otherF;
          //    }
          //  }
          //}
        }

        bool PdxInstanceImpl::deepArrayEquals(Object^ obj, Object^ otherObj)
        {
          if (obj == nullptr && otherObj == nullptr)
            return true;
          else if (obj == nullptr && otherObj != nullptr)
            return false;
          else if (obj != nullptr && otherObj == nullptr)
            return false;

          Type^ objT = obj->GetType();
          Type^ otherObjT = otherObj->GetType();
          if (!objT->Equals(otherObjT))
            return false;

          if (objT->IsArray)
          {//array
            return enumerableEquals((System::Collections::IEnumerable^)obj, (System::Collections::IEnumerable^)otherObj);
          }
          else if (objT->GetInterface("System.Collections.IDictionary"))
          {//map
            // Log::Debug(" in map");
            return enumerateDictionaryForEqual((System::Collections::IDictionary^)obj, (System::Collections::IDictionary^)otherObj);
          }
          else if (objT->GetInterface("System.Collections.IList"))
          {//list
            // Log::Debug(" in list");
            return enumerableEquals((System::Collections::IEnumerable^)obj, (System::Collections::IEnumerable^)otherObj);
          }
          else
          {

            //  Log::Debug("final object hashcode " + obj->GetHashCode());

            return obj->Equals(otherObj);
          }
        }

        bool PdxInstanceImpl::enumerableEquals(System::Collections::IEnumerable^ enumObj, System::Collections::IEnumerable^ enumOtherObj)
        {
          if (enumObj == nullptr && enumOtherObj == nullptr)
            return true;
          else if (enumObj == nullptr && enumOtherObj != nullptr)
            return false;
          else if (enumObj != nullptr && enumOtherObj == nullptr)
            return false;


          System::Collections::IEnumerator^ my = enumObj->GetEnumerator();
          System::Collections::IEnumerator^ other = enumOtherObj->GetEnumerator();


          while (true)
          {
            bool m = my->MoveNext();
            bool o = other->MoveNext();
            if (m && o)
            {
              if (!my->Current->Equals(other->Current))
                return false;
            }
            else if (!m && !o)
              return true;
            else
              return false;
          }
          // Log::Debug(" in enumerableHashCode FINAL hc " + h);
          return true;
        }

        bool PdxInstanceImpl::enumerateDictionaryForEqual(System::Collections::IDictionary^ iDict, System::Collections::IDictionary^ otherIDict)
        {
          if (iDict == nullptr && otherIDict == nullptr)
            return true;
          else if (iDict == nullptr && otherIDict != nullptr)
            return false;
          else if (iDict != nullptr && otherIDict == nullptr)
            return false;

          if (iDict->Count != otherIDict->Count)
            return false;

          System::Collections::IDictionaryEnumerator^ dEnum = iDict->GetEnumerator();
          for each(System::Collections::DictionaryEntry^ de in iDict)
          {
            Object^ other = nullptr;
            if (otherIDict->Contains(de->Key))
            {
              if (!deepArrayEquals(de->Value, otherIDict[de->Key]))
                return false;
            }
            else
              return false;
          }
          // Log::Debug(" in enumerateDictionary FINAL hc " + h);
          return true;
        }



        int PdxInstanceImpl::GetHashCode()
        {
          int hashCode = 1;

          PdxType^ pt = getPdxType();

          IList<PdxFieldType^>^ pdxIdentityFieldList = getIdentityPdxFields(pt);

          DataInput^ dataInput = gcnew DataInput(m_buffer, m_bufferLength, m_cache);
          dataInput->setPdxdeserialization(true);

          for (int i = 0; i < pdxIdentityFieldList->Count; i++)
          {
            PdxFieldType^ pField = pdxIdentityFieldList[i];

            //Log::Debug("hashcode for pdxfield " + pField->FieldName + " hashcode is " + hashCode);
            switch (pField->TypeId)
            {
            case PdxFieldTypes::CHAR:
            case PdxFieldTypes::BOOLEAN:
            case PdxFieldTypes::BYTE:
            case PdxFieldTypes::SHORT:
            case PdxFieldTypes::INT:
            case PdxFieldTypes::LONG:
            case PdxFieldTypes::DATE:
            case PdxFieldTypes::FLOAT:
            case PdxFieldTypes::DOUBLE:
            case PdxFieldTypes::STRING:
            case PdxFieldTypes::BOOLEAN_ARRAY:
            case PdxFieldTypes::CHAR_ARRAY:
            case PdxFieldTypes::BYTE_ARRAY:
            case PdxFieldTypes::SHORT_ARRAY:
            case PdxFieldTypes::INT_ARRAY:
            case PdxFieldTypes::LONG_ARRAY:
            case PdxFieldTypes::FLOAT_ARRAY:
            case PdxFieldTypes::DOUBLE_ARRAY:
            case PdxFieldTypes::STRING_ARRAY:
            case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
            {
              int retH = getRawHashCode(pt, pField, dataInput);
              if (retH != 0)
                hashCode = 31 * hashCode + retH;
              break;
            }
            case PdxFieldTypes::OBJECT:
            {
              setOffsetForObject(dataInput, pt, pField->SequenceId);
              Object^ object = dataInput->ReadObject();

              if (object != nullptr)
              {
                if (object->GetType()->IsArray)
                {
                  if (object->GetType()->GetElementType()->IsPrimitive)//primitive type
                  {
                    int retH = getRawHashCode(pt, pField, dataInput);
                    if (retH != 0)
                      hashCode = 31 * hashCode + retH;
                  }
                  else//array of objects
                  {
                    hashCode = 31 * hashCode + deepArrayHashCode(object);
                  }
                }
                else//object but can be hashtable, list etc
                {
                  hashCode = 31 * hashCode + deepArrayHashCode(object);
                }
              }
              else
              {
                //hashCode = 31 * hashCode; // this may be issue 
              }

              break;
            }
            case PdxFieldTypes::OBJECT_ARRAY:
            {
              setOffsetForObject(dataInput, pt, pField->SequenceId);
              Object^ objectArray = dataInput->ReadObjectArray();
              hashCode = 31 * hashCode + (objectArray != nullptr) ? deepArrayHashCode(objectArray) : 0;
              break;
            }
            default:
            {
              throw gcnew IllegalStateException("PdxInstance not found typeid " + pField->TypeId);
            }
            }

          }
          return hashCode;
        }


        int PdxInstanceImpl::deepArrayHashCode(Object^ obj)
        {
          if (obj == nullptr)
            return 0;

          Type^ objT = obj->GetType();

          /*for each(Type^ tmp in objT->GetInterfaces())
            //Log::Debug("interfaces " + tmp);*/

          if (objT->IsArray)
          {//array
            //if(objT->GetElementType()->IsPrimitive)
            //{//primitive array
            //  return primitiveArrayHashCode((array<int>^)obj);
            //}
            //else
            {//object array
              return enumerableHashCode((System::Collections::IEnumerable^)obj);
            }
          }
          else if (objT->GetInterface("System.Collections.IDictionary"))
          {//map
            // Log::Debug(" in map");
            return enumerateDictionary((System::Collections::IDictionary^)obj);
          }
          else if (objT->GetInterface("System.Collections.IList"))
          {//list
            // Log::Debug(" in list");
            return enumerableHashCode((System::Collections::IEnumerable^)obj);
          }
          else
          {

            //  Log::Debug("final object hashcode " + obj->GetHashCode());

            if (obj->GetType()->Equals(DotNetTypes::BooleanType))
            {
              if ((bool)obj)
                return 1231;
              else
                return 1237;
            }
            else if (obj->GetType()->Equals(DotNetTypes::StringType))
            {
              String^ str = (String^)obj;
              int prime = 31;
              int h = 0;
              for (int i = 0; i < str->Length; i++)
                h = prime*h + str[i];
              return h;
            }

            return obj->GetHashCode();
          }
        }

        int PdxInstanceImpl::enumerableHashCode(System::Collections::IEnumerable^ enumObj)
        {
          int h = 1;
          for each(Object^ o in enumObj)
          {
            h = h * 31 + deepArrayHashCode(o);
            //  Log::Debug(" in enumerableHashCode hc " + h);
          }
          // Log::Debug(" in enumerableHashCode FINAL hc " + h);
          return h;
        }

        int PdxInstanceImpl::enumerateDictionary(System::Collections::IDictionary^ iDict)
        {
          int h = 0;
          System::Collections::IDictionaryEnumerator^ dEnum = iDict->GetEnumerator();
          for each(System::Collections::DictionaryEntry^ de in iDict)
          {
            //System::Collections::DictionaryEntry^ de = (System::Collections::DictionaryEntry^)o;
            h = h + ((deepArrayHashCode(de->Key)) ^ ((de->Value != nullptr) ? deepArrayHashCode(de->Value) : 0));
          }
          // Log::Debug(" in enumerateDictionary FINAL hc " + h);
          return h;
        }

        generic <class T>
          int PdxInstanceImpl::primitiveArrayHashCode(T objArray)
          {
            if (objArray == nullptr)
              return 0;

            bool isBooleanType = false;
            if (objArray->Count > 0 && objArray->GetType()->GetElementType()->Equals(DotNetTypes::BooleanType))
              isBooleanType = true;

            //Log::Debug("primitiveArrayHashCode isbool " + isBooleanType);
            int h = 1;
            for each(Object^ o in objArray)
            {
              if (isBooleanType)
              {
                if ((bool)o)
                  h = h * 31 + 1231;
                else
                  h = h * 31 + 1237;
              }
              else
                h = h * 31 + o->GetHashCode();
            }

            // Log::Debug(" primitiveArrayHashCode final hc " + h);

            return h;
          }

          int PdxInstanceImpl::getRawHashCode(PdxType^ pt, PdxFieldType^ pField, DataInput^ dataInput)
          {
            int pos = getOffset(dataInput, pt, pField->SequenceId);
            int nextpos = getNextFieldPosition(dataInput, pField->SequenceId + 1, pt);

            if (hasDefaultBytes(pField, dataInput, pos, nextpos))
              return 0;//matched default bytes

            dataInput->ResetAndAdvanceCursorPdx(nextpos - 1);

            int h = 1;
            for (int i = nextpos - 1; i >= pos; i--)
            {
              h = 31 * h + (int)dataInput->ReadSByte();
              dataInput->ResetAndAdvanceCursorPdx(i - 1);
            }
            //Log::Debug("getRawHashCode nbytes " + (nextpos - pos) + " final hashcode" + h);
            return h;
          }

          bool PdxInstanceImpl::compareDefaulBytes(DataInput^ dataInput, int start, int end, array<SByte>^ defaultBytes)
          {
            if ((end - start) != defaultBytes->Length)
              return false;

            dataInput->ResetAndAdvanceCursorPdx(start);
            int j = 0;
            for (int i = start; i < end; i++)
            {
              if (defaultBytes[j++] != dataInput->ReadSByte())
              {
                return false;
              }
            }
            return true;
          }

          bool PdxInstanceImpl::hasDefaultBytes(PdxFieldType^ pField, DataInput^ dataInput, int start, int end)
          {
            switch (pField->TypeId)
            {
            case PdxFieldTypes::INT:
            {
              return compareDefaulBytes(dataInput, start, end, Int_DefaultBytes);
            }
            case PdxFieldTypes::STRING:
            {
              return compareDefaulBytes(dataInput, start, end, String_DefaultBytes);
            }
            case PdxFieldTypes::BOOLEAN:
            {
              return compareDefaulBytes(dataInput, start, end, Boolean_DefaultBytes);
            }
            case PdxFieldTypes::FLOAT:
            {
              return compareDefaulBytes(dataInput, start, end, Float_DefaultBytes);
            }
            case PdxFieldTypes::DOUBLE:
            {
              return compareDefaulBytes(dataInput, start, end, Double_DefaultBytes);
            }
            case PdxFieldTypes::CHAR:
            {
              return compareDefaulBytes(dataInput, start, end, Char_DefaultBytes);
            }
            case PdxFieldTypes::BYTE:
            {
              return compareDefaulBytes(dataInput, start, end, Byte_DefaultBytes);
            }
            case PdxFieldTypes::SHORT:
            {
              return compareDefaulBytes(dataInput, start, end, Short_DefaultBytes);
            }
            case PdxFieldTypes::LONG:
            {
              return compareDefaulBytes(dataInput, start, end, Long_DefaultBytes);
            }
            case PdxFieldTypes::BYTE_ARRAY:
            case PdxFieldTypes::DOUBLE_ARRAY:
            case PdxFieldTypes::FLOAT_ARRAY:
            case PdxFieldTypes::SHORT_ARRAY:
            case PdxFieldTypes::INT_ARRAY:
            case PdxFieldTypes::LONG_ARRAY:
            case PdxFieldTypes::BOOLEAN_ARRAY:
            case PdxFieldTypes::CHAR_ARRAY:
            case PdxFieldTypes::STRING_ARRAY:
            case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
            case PdxFieldTypes::OBJECT_ARRAY:
            {
              return compareDefaulBytes(dataInput, start, end, NULL_ARRAY_DefaultBytes);
            }
            case PdxFieldTypes::DATE:
            {
              return compareDefaulBytes(dataInput, start, end, Date_DefaultBytes);
            }
            case PdxFieldTypes::OBJECT:
            {
              return compareDefaulBytes(dataInput, start, end, Object_DefaultBytes);
            }
            default://object
            {
              throw gcnew IllegalStateException("hasDefaultBytes unable to find typeID  " + pField->TypeId);
            }
            }
          }

          bool PdxInstanceImpl::isPrimitiveArray(Object^ object)
          {
            Type^ type = object->GetType();

            if (type->IsArray)
            {
              return type->GetElementType()->IsPrimitive;
            }
            return false;
          }

          IList<PdxFieldType^>^ PdxInstanceImpl::getIdentityPdxFields(PdxType^ pt)
          {
            System::Comparison<PdxFieldType^>^ cd = gcnew System::Comparison<PdxFieldType^>(PdxInstanceImpl::comparePdxField);
            IList<PdxFieldType^>^ pdxFieldList = pt->PdxFieldList;
            List<PdxFieldType^>^ retList = gcnew List<PdxFieldType^>();

            for (int i = 0; i < pdxFieldList->Count; i++)
            {
              PdxFieldType^ pft = pdxFieldList[i];
              if (pft->IdentityField)
                retList->Add(pft);
            }

            if (retList->Count > 0)
            {
              retList->Sort(cd);
              return retList;
            }

            for (int i = 0; i < pdxFieldList->Count; i++)
            {
              PdxFieldType^ pft = pdxFieldList[i];
              retList->Add(pft);
            }

            retList->Sort(cd);
            return retList;
          }

          int PdxInstanceImpl::comparePdxField(PdxFieldType^ a, PdxFieldType^ b)
          {
            return a->FieldName->CompareTo(b->FieldName);
          }

          String^ PdxInstanceImpl::ToString()
          {
            PdxType^ pt = getPdxType();

            StringBuilder^ result = gcnew StringBuilder();
            result->Append("PDX[")->Append(pt->TypeId)->Append(",")->Append(pt->PdxClassName)
              ->Append("]{");
            bool firstElement = true;
            for each(PdxFieldType^ fieldType in getIdentityPdxFields(pt))
            {
              if (firstElement)
              {
                firstElement = false;
              }
              else
              {
                result->Append(", ");
              }
              result->Append(fieldType->FieldName);
              result->Append("=");
              try
              {
                // TODO check to see if getField returned an array and if it did use Arrays.deepToString
                result->Append(GetField(fieldType->FieldName));
              }
              catch (System::Exception^ e)
              {
                result->Append(e->Message);
              }
            }
            result->Append("}");
            return result->ToString();
          }

          IWritablePdxInstance^ PdxInstanceImpl::CreateWriter()
          {
            //dataInput->ResetPdx(0);
            return gcnew PdxInstanceImpl(m_buffer, m_bufferLength, m_typeId, false, m_cache);//need to create duplicate byte stream
          }

          void PdxInstanceImpl::SetField(String^ fieldName, Object^ value)
          {
            PdxType^ pt = getPdxType();
            PdxFieldType^ pft = pt->GetPdxField(fieldName);

            if (pft != nullptr && checkType(value->GetType(), pft->TypeId))//TODO::need to check typeas well
            {
              if (m_updatedFields == nullptr)
              {
                m_updatedFields = gcnew Dictionary<String^, Object^>();
              }
              m_updatedFields[fieldName] = value;
              return;
            }

            throw gcnew IllegalStateException("PdxInstance doesn't has field " + fieldName + " or type of field not matched " + (pft != nullptr ? pft->ToString() : ""));
          }

          void PdxInstanceImpl::ToData(IPdxWriter^ writer)
          {
            PdxType^ pt = getPdxType();

            IList<PdxFieldType^>^ pdxFieldList = pt->PdxFieldList;

            int position = 0;//ignore typeid and length
            int nextFieldPosition;

            if (m_buffer != NULL)
            {
              System::Byte* copy = m_buffer;

              if (!m_own)
                copy = apache::geode::client::DataInput::getBufferCopy(m_buffer, m_bufferLength);

              DataInput^ dataInput = gcnew DataInput(copy, m_bufferLength, m_cache);//this will delete buffer
              dataInput->setPdxdeserialization(true);
              //but new stream is set for this from pdxHelper::serialize function

              for (int i = 0; i < pdxFieldList->Count; i++)
              {
                PdxFieldType^ currPf = pdxFieldList[i];

                Object^ value = nullptr;
                m_updatedFields->TryGetValue(currPf->FieldName, value);
                //Log::Debug("field name " + currPf->FieldName);
                if (value != nullptr)
                {//
                  //Log::Debug("field updating " + value);
                  writeField(writer, currPf->FieldName, currPf->TypeId, value);
                  position = getNextFieldPosition(dataInput, i + 1, pt);
                }
                else
                {
                  if (currPf->IsVariableLengthType)
                  {//need to add offset
                    (static_cast<PdxLocalWriter^>(writer))->AddOffset();
                  }

                  //write raw byte array...
                  nextFieldPosition = getNextFieldPosition(dataInput, i + 1, pt);

                  writeUnmodifieldField(dataInput, position, nextFieldPosition, static_cast<PdxLocalWriter^>(writer));

                  position = nextFieldPosition;//mark next field;
                }
              }
            }
            else
            {
              for (int i = 0; i < pdxFieldList->Count; i++)
              {
                PdxFieldType^ currPf = pdxFieldList[i];

                Object^ value = m_updatedFields[currPf->FieldName];

                //Log::Debug("field updating " + value);
                writeField(writer, currPf->FieldName, currPf->TypeId, value);
              }
            }

            m_updatedFields->Clear();

            //now update the raw data...which will happen in PdxHelper
          }

          void PdxInstanceImpl::cleanup()
          {
            if (m_own)
            {
              m_own = false;
              apache::geode::client::DataOutput::safeDelete(m_buffer);
            }
          }

          void PdxInstanceImpl::updatePdxStream(System::Byte* newPdxStream, int len)
          {
            m_buffer = newPdxStream;
            m_own = true;
            m_bufferLength = len;
          }

          void PdxInstanceImpl::writeUnmodifieldField(DataInput^ dataInput, int startPos, int endPos, PdxLocalWriter^ localWriter)
          {
            //Log::Debug("writeUnmodifieldField startpos " + startPos + " endpos " + endPos);
            dataInput->ResetPdx(startPos);
            for (; startPos < endPos; startPos++)
            {
              localWriter->WriteByte(dataInput->ReadByte());
            }
          }

          int PdxInstanceImpl::getNextFieldPosition(DataInput^ dataInput, int fieldId, PdxType^ pt)
          {
            if (fieldId == pt->Totalfields)
            {//return serialized length
              return getSerializedLength(dataInput, pt);
            }
            else
            {
              return getOffset(dataInput, pt, fieldId);
            }
          }

          void PdxInstanceImpl::FromData(IPdxReader^ reader)
          {
            throw gcnew IllegalStateException("PdxInstance::FromData( .. ) shouldn't have called");
          }

          PdxType^ PdxInstanceImpl::getPdxType()
          {
            if (m_typeId == 0)
            {
              if (m_pdxType == nullptr)
              {
                throw gcnew IllegalStateException("PdxType should not be null..");
              }
              return m_pdxType;
            }

            PdxType^ pType = m_cache->GetPdxTypeRegistry()->GetPdxType(m_typeId);

            return pType;
          }

          void PdxInstanceImpl::setPdxId(Int32 typeId)
          {
            if (m_typeId == 0)
            {
              m_typeId = typeId;
              m_pdxType = nullptr;
            }
            else
            {
              throw gcnew IllegalStateException("PdxInstance's typeId is already set.");
            }
          }

          Object^ PdxInstanceImpl::readField(DataInput^ dataInput, String^ fieldName, int typeId)
          {
            switch (typeId)
            {
            case PdxFieldTypes::INT:
            {
              return dataInput->ReadInt32();
            }
            case PdxFieldTypes::STRING:
            {
              return dataInput->ReadString();
            }
            case PdxFieldTypes::BOOLEAN:
            {
              return dataInput->ReadBoolean();
            }
            case PdxFieldTypes::FLOAT:
            {
              return dataInput->ReadFloat();
            }
            case PdxFieldTypes::DOUBLE:
            {
              return dataInput->ReadDouble();
            }
            case PdxFieldTypes::CHAR:
            {
              return dataInput->ReadChar();
            }
            case PdxFieldTypes::BYTE:
            {
              return dataInput->ReadSByte();
            }
            case PdxFieldTypes::SHORT:
            {
              return dataInput->ReadInt16();
            }
            case PdxFieldTypes::LONG:
            {
              return dataInput->ReadInt64();
            }
            case PdxFieldTypes::BYTE_ARRAY:
            {
              return dataInput->ReadBytes();
            }
            case PdxFieldTypes::DOUBLE_ARRAY:
            {
              return dataInput->ReadDoubleArray();
            }
            case PdxFieldTypes::FLOAT_ARRAY:
            {
              return dataInput->ReadFloatArray();
            }
            case PdxFieldTypes::SHORT_ARRAY:
            {
              return dataInput->ReadShortArray();
            }
            case PdxFieldTypes::INT_ARRAY:
            {
              return dataInput->ReadIntArray();
            }
            case PdxFieldTypes::LONG_ARRAY:
            {
              return dataInput->ReadLongArray();
            }
            case PdxFieldTypes::BOOLEAN_ARRAY:
            {
              return dataInput->ReadBooleanArray();
            }
            case PdxFieldTypes::CHAR_ARRAY:
            {
              return dataInput->ReadCharArray();
            }
            case PdxFieldTypes::STRING_ARRAY:
            {
              return dataInput->ReadStringArray();
            }
            case PdxFieldTypes::DATE:
            {
              return dataInput->ReadDate();
            }
            case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
            {
              return dataInput->ReadArrayOfByteArrays();
            }
            case PdxFieldTypes::OBJECT_ARRAY:
            {
              return dataInput->ReadObjectArray();
            }
            default://object
            {
              return dataInput->ReadObject();
              //throw gcnew IllegalStateException("ReadField unable to de-serialize  " 
              //																	+ fieldName + " of " + type); 
            }
            }
          }

          bool PdxInstanceImpl::checkType(Type^ type, int typeId)
          {
            // Log::Fine("PdxInstanceImpl::checkType1 " + type->ToString() + "  " + typeId); 
            switch (typeId)
            {
            case PdxFieldTypes::INT:
            {
              // Log::Fine("PdxInstanceImpl::checkType " + type->ToString() + " : " +DotNetTypes::IntType->ToString());
              return type->Equals(DotNetTypes::IntType);
            }
            case PdxFieldTypes::STRING:
            {
              return type->Equals(DotNetTypes::StringType);
            }
            case PdxFieldTypes::BOOLEAN:
            {
              return type->Equals(DotNetTypes::BooleanType);
            }
            case PdxFieldTypes::FLOAT:
            {
              return type->Equals(DotNetTypes::FloatType);
            }
            case PdxFieldTypes::DOUBLE:
            {
              return type->Equals(DotNetTypes::DoubleType);
            }
            case PdxFieldTypes::CHAR:
            {
              return type->Equals(DotNetTypes::CharType);
            }
            case PdxFieldTypes::BYTE:
            {
              return type->Equals(DotNetTypes::SByteType);
            }
            case PdxFieldTypes::SHORT:
            {
              return type->Equals(DotNetTypes::ShortType);
            }
            case PdxFieldTypes::LONG:
            {
              return type->Equals(DotNetTypes::LongType);
            }
            case PdxFieldTypes::BYTE_ARRAY:
            {
              return type->Equals(DotNetTypes::ByteArrayType);
            }
            case PdxFieldTypes::DOUBLE_ARRAY:
            {
              return type->Equals(DotNetTypes::DoubleArrayType);
            }
            case PdxFieldTypes::FLOAT_ARRAY:
            {
              return type->Equals(DotNetTypes::FloatArrayType);
            }
            case PdxFieldTypes::SHORT_ARRAY:
            {
              return type->Equals(DotNetTypes::ShortArrayType);
            }
            case PdxFieldTypes::INT_ARRAY:
            {
              return type->Equals(DotNetTypes::IntArrayType);
            }
            case PdxFieldTypes::LONG_ARRAY:
            {
              return type->Equals(DotNetTypes::LongArrayType);
            }
            case PdxFieldTypes::BOOLEAN_ARRAY:
            {
              return type->Equals(DotNetTypes::BoolArrayType);
            }
            case PdxFieldTypes::CHAR_ARRAY:
            {
              return type->Equals(DotNetTypes::CharArrayType);
            }
            case PdxFieldTypes::STRING_ARRAY:
            {
              return type->Equals(DotNetTypes::StringArrayType);
            }
            case PdxFieldTypes::DATE:
            {
              return type->Equals(DotNetTypes::DateType);
            }
            case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
            {
              return type->Equals(DotNetTypes::ByteArrayOfArrayType);
            }
            case PdxFieldTypes::OBJECT_ARRAY:
            {
              return type->Equals(DotNetTypes::ObjectArrayType);
            }
            default://object
            {
              return true;
              //throw gcnew IllegalStateException("ReadField unable to de-serialize  " 
              //																	+ fieldName + " of " + type); 
            }
            }
          }

          void PdxInstanceImpl::writeField(IPdxWriter^ writer, String^ fieldName, int typeId, Object^ value)
          {
            switch (typeId)
            {
            case PdxFieldTypes::INT:
            {
              writer->WriteInt(fieldName, (int)value);
              break;
            }
            case PdxFieldTypes::STRING:
            {
              writer->WriteString(fieldName, (String^)value);
              break;
            }
            case PdxFieldTypes::BOOLEAN:
            {
              writer->WriteBoolean(fieldName, (bool)value);
              break;
            }
            case PdxFieldTypes::FLOAT:
            {
              writer->WriteFloat(fieldName, (float)value);
              break;
            }
            case PdxFieldTypes::DOUBLE:
            {
              writer->WriteDouble(fieldName, (double)value);
              break;
            }
            case PdxFieldTypes::CHAR:
            {
              writer->WriteChar(fieldName, (Char)value);
              break;
            }
            case PdxFieldTypes::BYTE:
            {
              writer->WriteByte(fieldName, (SByte)value);
              break;
            }
            case PdxFieldTypes::SHORT:
            {
              writer->WriteShort(fieldName, (short)value);
              break;
            }
            case PdxFieldTypes::LONG:
            {
              writer->WriteLong(fieldName, (Int64)value);
              break;
            }
            case PdxFieldTypes::BYTE_ARRAY:
            {
              writer->WriteByteArray(fieldName, (array<Byte>^)value);
              break;
            }
            case PdxFieldTypes::DOUBLE_ARRAY:
            {
              writer->WriteDoubleArray(fieldName, (array<double>^)value);
              break;
            }
            case PdxFieldTypes::FLOAT_ARRAY:
            {
              writer->WriteFloatArray(fieldName, (array<float>^)value);
              break;
            }
            case PdxFieldTypes::SHORT_ARRAY:
            {
              writer->WriteShortArray(fieldName, (array<short>^)value);
              break;
            }
            case PdxFieldTypes::INT_ARRAY:
            {
              writer->WriteIntArray(fieldName, (array<int>^)value);
              break;
            }
            case PdxFieldTypes::LONG_ARRAY:
            {
              writer->WriteLongArray(fieldName, (array<Int64>^)value);
              break;
            }
            case PdxFieldTypes::BOOLEAN_ARRAY:
            {
              writer->WriteBooleanArray(fieldName, (array<bool>^)value);
              break;
            }
            case PdxFieldTypes::CHAR_ARRAY:
            {
              writer->WriteCharArray(fieldName, (array<Char>^)value);
              break;
            }
            case PdxFieldTypes::STRING_ARRAY:
            {
              writer->WriteStringArray(fieldName, (array<String^>^)value);
              break;
            }
            case PdxFieldTypes::DATE:
            {
              writer->WriteDate(fieldName, (DateTime)value);
              break;
            }
            case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
            {
              writer->WriteArrayOfByteArrays(fieldName, (array<array<Byte>^>^)value);
              break;
            }
            case PdxFieldTypes::OBJECT_ARRAY:
            {
              writer->WriteObjectArray(fieldName, (List<Object^>^)value);
              break;
            }
            default:
            {
              writer->WriteObject(fieldName, value);
            }
          }
        }
      }  // namespace Internal
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
