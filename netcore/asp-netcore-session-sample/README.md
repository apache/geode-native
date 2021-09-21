You can configure GeodeDistributedCache as a distributed cache for your ASP.NET Core application using either of the options described below.

---
NetCore-Session provides the AddGeodeDistributedCache() extension method on IServiceCollection, which requires just an Apache Geode region name and any optional configurations to store the sessions. 

There are two methods to specify configurations:
1. Through your application in Startup.cs or
2. In JSON format in Appsettings.json of your application.
   
## Method 1: Specifying Configurations In Startup.cs

The AddGeodeSessionStateCache() method is an extension of the AddDistributedCache() method provided by ASP.NET Core. This method takes configuration settings in Startup.cs of your application, or reads them from the specified JSON file.

Add the following method and options in Startup.cs of your application:

```c#
public void ConfigureServices(IServiceCollection services)
{
    //Add framework services
    services.AddMvc();

    services.AddGeodeSessionStateCache(configuration =>
    {
        configuration.RegionName = "geodeSessionState";
    });
}
```

## Method 2: Specifying Configurations In appsettings.json
You can also specify the configuration in JSON format using the appsettings.json file for your application. Using this method, you can refer to the configurations by providing the name of the section containing JSON format configurations in Startup.cs.

appsettings.json:

```json
{
  "AppSettings": {
    "SiteTitle": "ASP.NET Core SessionState Sample App"
  },

  "GeodeSessionStateCache": {
    "RegionName": "geodeSessionState",
    "Host": "localhost",
    "Port": 10334
  }
}
```

The ConfigureServices member of the Startup class can be used as follows to retrieve the GeodeSessionStateCache settings from the JSON file:

```c#
public void ConfigureServices(IServiceCollection services)
{
  ...
  services.Configure<GeodeSessionStateCacheOptions>(
    Configuration.GetSection("GeodeSessionStateCache"));
  ...
}
```
