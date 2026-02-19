Add-Type -TypeDefinition @"
using System;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct GeomIPCBlock {
    public uint magic;
    public uint version;
    public uint cmd;
    public uint seqNo;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 512)]
    public string geomFilePath;
    [MarshalAs(managedType: UnmanagedType.ByValTStr, SizeConst = 512)]
    public string resultFilePath;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
    public string errorMsg;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 256)]
    public byte[] _pad;
}

public class GeomIPC {
    public static void Check() {
        try {
            using (var mmf = MemoryMappedFile.OpenExisting("GeomIPC_v1")) {
                using (var accessor = mmf.CreateViewAccessor(0, 2048)) {
                    byte[] buffer = new byte[2048];
                    accessor.ReadArray<byte>(0, buffer, 0, 2048);
                    IntPtr ptr = Marshal.AllocHGlobal(2048);
                    Marshal.Copy(buffer, 0, ptr, 2048);
                    GeomIPCBlock block = (GeomIPCBlock)Marshal.PtrToStructure(ptr, typeof(GeomIPCBlock));
                    Marshal.FreeHGlobal(ptr);
                    Console.WriteLine("Shared memory found:");
                    Console.WriteLine($"  magic: 0x{block.magic:X}");
                    Console.WriteLine($"  version: {block.version}");
                    Console.WriteLine($"  cmd: {block.cmd}");
                    Console.WriteLine($"  seqNo: {block.seqNo}");
                    Console.WriteLine($"  geomFilePath: {block.geomFilePath}");
                    Console.WriteLine($"  resultFilePath: {block.resultFilePath}");
                    Console.WriteLine($"  errorMsg: {block.errorMsg}");
                }
            }
        } catch (Exception ex) {
            Console.WriteLine($"Failed to open shared memory: {ex.Message}");
        }
    }
}
"@ -ErrorAction SilentlyContinue

[GeomIPC]::Check()