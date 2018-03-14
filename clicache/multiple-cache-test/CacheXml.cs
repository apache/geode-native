using System;
using System.IO;
using System.Diagnostics;

public class CacheXml : IDisposable
{
    public FileInfo File
    {
        get;
        private set;
    }

    public CacheXml(FileInfo template, GemFireServer gfs, string regionName = "")
    {
        string content;

        using (var input = template.OpenText())
        {
            content = input.ReadToEnd();
        }

        content = content.Replace("LOCATOR_PORT", gfs.LocatorPort.ToString());
        content = content.Replace("REGION_NAME", !string.IsNullOrEmpty(regionName) ? regionName : "session");

        Debug.WriteLine(content);

        var tempFile = new FileInfo(Path.GetTempFileName()) { Attributes = FileAttributes.Temporary };

        // Set the Attribute property of this file to Temporary. 
        // Although this is not completely necessary, the .NET Framework is able 
        // to optimize the use of Temporary files by keeping them cached in memory.
        using (var output = new StreamWriter(tempFile.FullName))
        {
            output.Write(content);
        }

        File = tempFile;
    }

    public void Dispose()
    {
        File.Delete();
    }
}
