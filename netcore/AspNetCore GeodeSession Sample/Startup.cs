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
using Apache.Geode.Session;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Hosting;
using SessionSample.Middleware;
using Microsoft.Extensions.Caching.Distributed;
using System;

namespace SessionSample {
  public class Startup {
    public Startup(IConfiguration configuration) {
      Configuration = configuration;
    }

    public IConfiguration Configuration { get; }
    public ILoggerFactory LoggerFactory { get; }

    public void ConfigureServices(IServiceCollection services) {
      services.Add(ServiceDescriptor.Singleton<IDistributedCache, GeodeSessionStateCache>());

      services.AddSession(options => {
        options.IdleTimeout = TimeSpan.FromSeconds(5);
        options.Cookie.HttpOnly = true;
        options.Cookie.IsEssential = true;
      });

      // Configure Method 1: Using extension method:
      services.AddGeodeSessionStateCache(configuration => {
        configuration.Host = "localhost";
        configuration.Port = 10334;
        configuration.RegionName = "geodeSessionState";
      });

      // Configure Method 2: Using appsettings.json:
      // services.Configure<GeodeSessionStateCacheOptions>(
      //    Configuration.GetSection("GeodeSessionStateCache"));

      services.AddControllersWithViews();
      services.AddRazorPages();
    }

    public void Configure(IApplicationBuilder app, IWebHostEnvironment env) {
      if (env.IsDevelopment()) {
        app.UseDeveloperExceptionPage();
      } else {
        app.UseExceptionHandler("/Home/Error");
        app.UseHsts();
      }

      app.UseStaticFiles();
      app.UseRouting();
      app.UseHttpContextItemsMiddleware();
      app.UseSession();
      app.UseEndpoints(endpoints => {
        endpoints.MapDefaultControllerRoute();
        endpoints.MapRazorPages();
      });
    }
  }
}
