using System;

class Program {
    static void Main() {
        ulong hash = 14695981039346656037ul;
        string s = "VIPOverlay.dds";
        foreach(char c in s) {
            hash ^= (byte)c;
            hash *= 1099511628211ul;
        }
        Console.WriteLine(hash);
    }
}
