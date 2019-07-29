/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include <iostream>
#include "ns3/core-module.h"
#include "ns3/config-store-module.h"

using namespace ns3;

class MyObject : public Object
{
public:
  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("MyObject")
      .SetParent (Object::GetTypeId ())
      .SetGroupName ("MyGroup")
      .AddConstructor<MyObject> ()
      .AddTraceSource ("MyInteger",
                       "An integer value to trace.",
                       MakeTraceSourceAccessor (&MyObject::m_myInt),
                       "ns3::TracedValueCallback::Int32")
      ;
    return tid;
  }

  MyObject () {}
  MyObject (int32_t v) {
    m_myInt = v;
  }
  TracedValue<int32_t> m_myInt;
};

void (* traceSink)(int32_t oldValue, int32_t newValue);

void
IntTrace (int32_t oldValue, int32_t newValue)
{
  std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
}

void
IntTrac (int32_t oldValue, int32_t newValue)
{
  std::cout << "From " << oldValue << " to " << newValue << std::endl;
}


int
main (int argc, char *argv[])
{
  MyObject myObject(3);
  MyObject myObject1(5);
  Ptr<MyObject> myO = CreateObject<MyObject>(myObject);
  Ptr<MyObject> myO1 = CreateObject<MyObject>(myObject1);
  myO->TraceConnectWithoutContext ("MyInteger", MakeCallback(&IntTrace));
  myO1->TraceConnectWithoutContext ("MyInteger", MakeCallback(&IntTrac));
  Config::Connect("/MyObject" , MakeCallback(&IntTrace));

  myO->m_myInt = 1234;
  myO1->m_myInt = 1234;
}
