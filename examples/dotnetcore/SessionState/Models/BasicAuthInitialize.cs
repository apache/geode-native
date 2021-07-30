using Apache.Geode.Client;
using System;
using System.Collections.Generic;

namespace GemFireSessionState.Models
{
  public class BasicAuthInitialize : IAuthInitialize
  {
    private string _username;
    private string _password;

    public BasicAuthInitialize(string username, string password)
    {
      _username = username;
      _password = password;
    }

    public void Close()
    {
    }

    public Dictionary<string, string> GetCredentials()
    {
      Console.WriteLine("SimpleAuthInitialize::GetCredentials called");
      var credentials = new Dictionary<string, string>();
      credentials.Add("security-username", "root");
      credentials.Add("security-password", "root-password");
      return credentials;
    }
  }
}