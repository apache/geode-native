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

namespace Apache.Geode.Examples.Serializer
{
  public class Order
  {
    public int OrderId { get; set; }
    public string Name { get; set; }
    public short Quantity { get; set; }

    // A default constructor is required for reflection based autoserialization
    public Order() { }

    public Order(int orderId, string name, short quantity)
    {
      OrderId = orderId;
      Name = name;
      Quantity = quantity;
    }

    public override string ToString()
    {
      return "Order: [" + OrderId + ", " + Name + ", " + Quantity + "]";
    }
  }
}


