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
    private String addressLine;
    private String city;
    private String country;
    private String instructions;
    private Vector phoneNumbers;

    public DeliveryAddress() {}
    public DeliveryAddress(String address, String city, String country, String instructions) {
      this.addressLine = address;
      this.city = city;
      this.country = country;
      this.instructions = instructions;
    }

    @Override
    public String toString() {
      return "DeliveryAddress[Address=" + addressLine + "; City=" + city +
         "; Country=" + country + "; Instructions=" + instructions +
         "; PhoneNumbers.isNull=" + (phoneNumbers == null ? "true" : "false") + "]";
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
      return addressLine.equals(other.addressLine) &&
             city.equals(other.city) &&
             country.equals(other.country) &&
             instructions.equals(other.instructions) &&
             (phoneNumbers == null) == (other.phoneNumbers == null);
    }

    
    public void fromData(PdxReader reader)
    {
      addressLine = reader.readString("address");
      city = reader.readString("city");
      country = reader.readString("country");
      instructions = reader.readString("instructions");
      phoneNumbers = (Vector)reader.readObject("phoneNumbers");
    }

    public void toData(PdxWriter writer)
    {
      writer.writeString("address", addressLine);
      writer.writeString("city", city);
      writer.writeString("country", country);
      writer.writeString("instructions", instructions);
      writer.writeObject("phoneNumbers", phoneNumbers);
    }
  }
