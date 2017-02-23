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

/*========================================================================
 *
 * Name - InterlockedCompareExchange -- 
 *
 * Purpose -
 * The function returns the initial value of the Destination parameter.
 * The function compares the Destination value with the Comparand value. 
 * If the Destination value is equal to the Comparand value, the Exchange 
 * value is stored in the address specified by Destination. 
 * Otherwise, no operation is performed.
 *
 * membar not used
 *
 *========================================================================
 */
      .seg    "text"
      .proc   12
      .global InterlockedCompareExchange
      .type InterlockedCompareExchange, #function
InterlockedCompareExchange:

       cas     [%o0],%o2,%o1   ! /* atomic CAS, with read value -> retval */
       retl
       mov     %o1,%o0         ! /* retval = 1 IFF tmp == 0 */

/*========================================================================
 *
 * Name - InterlockedExchangeAdd -- Gemfire name was HostAsmAtomicAdd
 *
 * Purpose -
 * Add 'increment' to  the counter pointed to be 'ctrPtr'.
 * Returns the old value.
 *
 * membar not used
 *
 *========================================================================
 */
      .proc   12
      .global InterlockedExchangeAdd 
      .type InterlockedExchangeAdd, #function
InterlockedExchangeAdd: 
                                   ! %o0 = ctrPtr 
                                   ! %o1 = increment
retryAdd:                          ! do {
        ld      [%o0],%o2          ! %o2 = *ctrPtr
        add     %o2,%o1,%o3        ! %o3 = %o2 + increment
        cas     [%o0],%o2,%o3      ! if (%o2 == *ctrPtr)
                                   !   tmp = *ctrPtr, *ctrPtr = %o3, %o3 = tmp
                                   ! else
                                   !   %o3 = *ctrPtr
        cmp     %o2,%o3            !
        bne     retryAdd           ! } while (%o2 != %o3)
        nop                        ! fix for bug 22851
        retl 
        mov     %o3,%o0            ! return old value of *ctrPtr in %o0

/*========================================================================
 *
 * Name - InterlockedExchangeAddLong -- Gemfire name was HostAsmAtomicAddLong
 *
 * Purpose -
 * Handels 64 bit Pointer for v9 architecture
 * Add 'increment' to  the counter pointed to be 'ctrPtr'.
 * Returns the old value.
 *
 * membar not used
 *
 *========================================================================
 */
      .proc   12
      .global InterlockedExchangeAddLong
      .type InterlockedExchangeAddLong, #function
InterlockedExchangeAddLong: 
                                   ! %o0 = ctrPtr 
                                   ! %o1 = increment
retryAddLong:                      ! do {
        ldx     [%o0],%o2          ! %o2 = *ctrPtr
        add     %o2,%o1,%o3        ! %o3 = %o2 + increment
        casx    [%o0],%o2,%o3      ! if (%o2 == *ctrPtr)
                                   !   tmp = *ctrPtr, *ctrPtr = %o3, %o3 = tmp
                                   ! else
                                   !   %o3 = *ctrPtr
        cmp     %o2,%o3            !
        bne     retryAddLong       ! } while (%o2 != %o3)
        nop                        ! fix for bug 22851
        retl 
        mov     %o3,%o0            ! return old value of *ctrPtr in %o0

