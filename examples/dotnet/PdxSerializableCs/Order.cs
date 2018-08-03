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
using Apache.Geode.Client;
using System;

namespace Apache.Geode.Examples.Serializer
{
    public class Order : IPdxSerializable
    {
        private const string ORDER_ID_KEY_ = "order_id";
        private const string NAME_KEY_ = "name";
        private const string QUANTITY_KEY_ = "quantity";

        public long OrderId { get; set; }
        public string Name { get; set; }
        public short Quantity { get; set; }

        // A default constructor is required for deserialization
        public Order() { }

        public Order(int orderId, string name, short quantity)
        {
            OrderId = orderId;
            Name = name;
            Quantity = quantity;
        }

        public override string ToString()
        {
            return string.Format("Order: [{0}, {1}, {2}]", OrderId, Name, Quantity);
        }

        public void ToData(IPdxWriter output)
        {
            output.WriteLong(ORDER_ID_KEY_, OrderId);
            output.MarkIdentityField(ORDER_ID_KEY_);

            output.WriteString(NAME_KEY_, Name);
            output.MarkIdentityField(NAME_KEY_);

            output.WriteInt(QUANTITY_KEY_, Quantity);
            output.MarkIdentityField(QUANTITY_KEY_);
        }

        public void FromData(IPdxReader input)
        {
            OrderId = input.ReadLong(ORDER_ID_KEY_);
            Name = input.ReadString(NAME_KEY_);
            Quantity = (short)input.ReadInt(QUANTITY_KEY_);
        }

        public Int32 ClassId
        {
            get { return 0x42; }
        }    

        public ulong ObjectSize
        {
            get { return 0x04; }
        }    
        
        public static IPdxSerializable CreateDeserializable()
        {
            return new Order();
        }
    }
}


