using System;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using SessionSample.Middleware;
using Web.Extensions;

namespace SessionSample.Pages
{
    public class IndexModel : PageModel
    {
        public const string SessionKeyName = "_Name";
        public const string SessionKeyAge = "_Age";
        const string SessionKeyTime = "_Time";

        public string SessionInfo_Name { get; private set; }
        public string SessionInfo_Age { get; private set; }
        public string SessionInfo_CurrentTime { get; private set; }
        public string SessionInfo_SessionTime { get; private set; }
        public string SessionInfo_MiddlewareValue { get; private set; }

        public void OnGet()
        {
            // Requires: using Microsoft.AspNetCore.Http;
            if (string.IsNullOrEmpty(HttpContext.Session.GetString(SessionKeyName)))
            {
                HttpContext.Session.SetString(SessionKeyName, "The Doctor");
                HttpContext.Session.SetInt32(SessionKeyAge, 35);
            }

            var name = HttpContext.Session.GetString(SessionKeyName);
            var age = HttpContext.Session.GetInt32(SessionKeyAge);
            SessionInfo_Name = name;
            SessionInfo_Age = age.ToString();

            var currentTime = DateTime.Now;

            // Requires SessionExtensions from sample download.
            if (HttpContext.Session.Get<DateTime>(SessionKeyTime) == default)
            {
                HttpContext.Session.Set<DateTime>(SessionKeyTime, currentTime);
            }

            SessionInfo_CurrentTime = currentTime.ToString("H:mm:ss tt");
            SessionInfo_SessionTime = HttpContext.Session.Get<DateTime>(SessionKeyTime)
                .ToString("H:mm:ss tt");

            HttpContext.Items
                .TryGetValue(HttpContextItemsMiddleware.HttpContextItemsMiddlewareKey, 
                    out var middlewareSetValue);
            SessionInfo_MiddlewareValue = 
                middlewareSetValue?.ToString() ?? "Middleware value not set!";
        }

        public IActionResult OnPostUpdateSessionDate()
        {
            HttpContext.Session.Set<DateTime>(SessionKeyTime, DateTime.Now);

            return RedirectToPage();
        }

        public IActionResult OnPostChangeAge()
        {
            var r = new Random();

            HttpContext.Session.SetInt32(SessionKeyAge, r.Next(35, 65));

            return RedirectToPage();
        }
    }
}
