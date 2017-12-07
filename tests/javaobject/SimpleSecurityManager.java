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
/*=========================================================================
* This implementation is provided on an "AS IS" BASIS,  WITHOUT WARRANTIES
* OR CONDITIONS OF ANY KIND, either express or implied."
*==========================================================================
*/

package javaobject;

import java.util.HashMap;
import java.util.Properties;

import org.apache.geode.management.internal.security.ResourceConstants;
import org.apache.geode.security.AuthenticationFailedException;
import org.apache.geode.security.ResourcePermission;
import org.apache.geode.security.SecurityManager;

public class SimpleSecurityManager implements SecurityManager {
  @Override
  public Object authenticate(Properties credentials) throws AuthenticationFailedException {
    String user = credentials.getProperty(ResourceConstants.USER_NAME);
    String password = credentials.getProperty(ResourceConstants.PASSWORD);

    if (getUserCredentials().containsKey(user) && getUserCredentials().get(user).equals(password)) {
        return user;
    }
    throw new AuthenticationFailedException("Non-authenticated user: " + user);
  }

  @Override
  public boolean authorize(Object principal, ResourcePermission permission) {
    String username = principal.toString();

    if (permission.getOperation() == ResourcePermission.Operation.MANAGE) {
        return username.equals("root");
    }

    if (permission.getOperation() == ResourcePermission.Operation.READ) {
        return username.contains("read") || username.equals("root");
    }

    if (permission.getOperation() == ResourcePermission.Operation.WRITE) {
        return username.contains("write") || username.equals("root");
    }

    return false;
  }

  private HashMap<String, String> getUserCredentials() {
    HashMap<String, String> userCredentials = new HashMap<String, String>();
    userCredentials.put("root", "root-password");
    userCredentials.put("reader", "reader-password");
    userCredentials.put("writer", "writer-password");
    return userCredentials;
  }
}
