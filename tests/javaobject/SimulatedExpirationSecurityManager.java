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
package javaobject;

import java.util.HashMap;
import java.util.Properties;

import org.apache.geode.management.internal.security.ResourceConstants;
import org.apache.geode.security.AuthenticationFailedException;
import org.apache.geode.security.AuthenticationExpiredException;
import org.apache.geode.security.ResourcePermission;
import org.apache.geode.security.SecurityManager;

import java.lang.management.ManagementFactory;
import java.util.Random;

import javaobject.UserPasswordAuthInit;
import javaobject.UsernamePrincipal;

/**
  This Security manager uses a random number generator to decide which users
  are authenticated. It is designed to force reauthentication for roughly
  one percent of the operations. Also, user "root" is always valid to allow
  executing gfsh commands during test setup.
*/
public class SimulatedExpirationSecurityManager implements SecurityManager {

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
  public boolean authorize(Object principal, ResourcePermission permission) throws AuthenticationExpiredException {

    // User "root" is allows authorized, and can be used to run gfsh commands to setup the cluster for testing.
    if (principal.toString() == "root")
      return true;

    // Throw AuthenticationExpiredException 1% of the time to allow testing expiration
    int numUsers = 100;
    Random rand = new Random();
    int userNumber = rand.nextInt(numUsers);

    if (userNumber < 1)
      throw new AuthenticationExpiredException("User authentication expired.");
    else
      return true;
  }

  private HashMap<String, String> getUserCredentials() {
    HashMap<String, String> userCredentials = new HashMap<String, String>();
    userCredentials.put("root", "root-password");
    userCredentials.put("user", "user-password");
    return userCredentials;
  }
}
