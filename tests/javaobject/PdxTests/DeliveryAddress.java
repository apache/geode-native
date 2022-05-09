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

package PdxTests;

import java.util.Vector;
import org.apache.geode.pdx.PdxReader;
import org.apache.geode.pdx.PdxSerializable;
import org.apache.geode.pdx.PdxWriter;

public class DeliveryAddress implements PdxSerializable
{
    String _addressLine;
    String _city;
    String _country;
    String _instructions;
    Vector _phoneNumbers;

    public DeliveryAddress() {}
    public DeliveryAddress(String address, String city, String country, String instructions) {
      _addressLine = address;
      _city = city;
      _country = country;
      _instructions = instructions;
    }

    @Override
    public String toString() {
      return "DeliveryAddress[Address=" + _addressLine + "; City=" + _city +
         "; Country=" + _country + "; Instructions=" + _instructions +
         "; PhoneNumbers.isNull=" + (_phoneNumbers == null ? "true" : "false") + "]";
    }

    public boolean equals(Object obj)
    {
      if (obj == null)
        return false;
      if(!(obj instanceof DeliveryAddress))
        return false;
      DeliveryAddress other = (DeliveryAddress)obj;
      if (other == null)
        return false;
      return _addressLine.equals(other._addressLine) &&
             _city.equals(other._city) &&
             _country.equals(other._country) &&
             _instructions.equals(other._instructions) &&
             (_phoneNumbers == null) == (other._phoneNumbers == null);
    }

    
    public void fromData(PdxReader reader)
    {
      _addressLine = reader.readString("address");
      _city = reader.readString("city");
      _country = reader.readString("country");
      _instructions = reader.readString("instructions");
      _phoneNumbers = (Vector)reader.readObject("phoneNumbers");
    }

    public void toData(PdxWriter writer)
    {
      writer.writeString("address", _addressLine);
      writer.writeString("city", _city);
      writer.writeString("country", _country);
      writer.writeString("instructions", _instructions);
      writer.writeObject("phoneNumbers", _phoneNumbers);
    }
  }
