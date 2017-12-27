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



#include "CqState.hpp"

using namespace System;
using namespace msclr::interop;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      String^ CqState::ToString()
      {
		  return marshal_as<String^>(m_nativeptr->toString());
      }

      bool CqState::IsRunning()
      {
        return m_nativeptr->isRunning();
      }

      bool CqState::IsStopped()
      {
        return m_nativeptr->isStopped();
      }

      bool CqState::IsClosed()
      {
        return m_nativeptr->isClosed();
      }

      bool CqState::IsClosing()
      {
        return m_nativeptr->isClosing();
      }

      void CqState::SetState( CqStateType state )
      {
	      apache::geode::client::CqState::StateType st =apache::geode::client::CqState::INVALID;
	      if(state == CqStateType::STOPPED)
		      st = apache::geode::client::CqState::STOPPED;
	      else if(state == CqStateType::RUNNING)
		      st = apache::geode::client::CqState::RUNNING;
	      else if(state == CqStateType::CLOSED)
		      st = apache::geode::client::CqState::CLOSED;
	      else if(state == CqStateType::CLOSING)
		      st = apache::geode::client::CqState::CLOSING;
      
        m_nativeptr->setState( st );
      }

      CqStateType CqState::GetState( )
      {
		    apache::geode::client::CqState::StateType st =  m_nativeptr->getState( );
            CqStateType state;
		    if(st==apache::geode::client::CqState::STOPPED)
			    state = CqStateType::STOPPED;
		    else if(st==apache::geode::client::CqState::RUNNING)
			    state = CqStateType::RUNNING;
		    else if(st==apache::geode::client::CqState::CLOSED)
			    state = CqStateType::CLOSED;
		    else if(st==apache::geode::client::CqState::CLOSING)
			    state = CqStateType::CLOSING;
		    else
			    state = CqStateType::INVALID;
		    return state;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
