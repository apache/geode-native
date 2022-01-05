using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace Apache.Geode.Client
{
  public class CBindings
  {
    public delegate void GetCredentialsDelegateInternal(IntPtr cache);
    public delegate void CloseDelegateInternal();

    [DllImport(Constants.libPath, CallingConvention = CallingConvention.Cdecl)]
    public static extern void apache_geode_CacheFactory_SetAuthInitialize(
        IntPtr factory, GetCredentialsDelegateInternal getCredentials,
        CloseDelegateInternal close);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_CreateCacheFactory();

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_DestroyCacheFactory(IntPtr factory);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_CacheFactory_GetVersion(IntPtr factory);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_CacheFactory_GetProductDescription(
        IntPtr factory);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(
        IntPtr factory, bool pdxIgnoreUnreadFields);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_CacheFactory_SetPdxReadSerialized(
        IntPtr factory, bool pdxReadSerialized);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_CacheFactory_SetProperty(IntPtr factory, IntPtr key,
                                                                     IntPtr value);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_CacheFactory_CreateCache(IntPtr factory);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern bool apache_geode_Cache_GetPdxIgnoreUnreadFields(IntPtr cache);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern bool apache_geode_Cache_GetPdxReadSerialized(IntPtr cache);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_Cache_GetName(IntPtr cache);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_Cache_Close(IntPtr cache, bool keepalive);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern bool apache_geode_Cache_IsClosed(IntPtr cache);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern bool apache_geode_AuthInitialize_AddProperty(IntPtr properties,
                                                                       IntPtr key,
                                                                       IntPtr value);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_DestroyCache(IntPtr cache);
    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern bool apache_geode_Region_PutString(IntPtr region, IntPtr key,
                                                             IntPtr value);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern bool apache_geode_Region_PutByteArray(IntPtr region, IntPtr key,
                                                                IntPtr value, int length);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_RegionFactory_CreateRegion(IntPtr cache,
                                                                         IntPtr regionName);
    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_Cache_CreateRegionFactory(IntPtr cache,
                                                                        int regionType);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_DestroyRegionFactory(IntPtr regionFactory);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_Region_GetByteArray(IntPtr region, IntPtr key,
                                                                ref IntPtr value, ref int size);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_Region_GetByteArrayForInt32Key(IntPtr region,
          [param: MarshalAs(UnmanagedType.I4)] Int32 key,
          ref IntPtr value, ref int size);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern void apache_geode_Region_Remove(IntPtr region, IntPtr key);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern bool apache_geode_Region_ContainsValueForKey(IntPtr region,
                                                                       IntPtr key);

    [DllImport(Constants.libPath, CharSet = CharSet.Auto)]
    public static extern IntPtr apache_geode_DestroyRegion(IntPtr region);
  }
}
