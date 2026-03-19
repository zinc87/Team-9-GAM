using System;
using System.IO;
using System.Text.RegularExpressions;

class Program
{
    static void Main()
    {
        string path = @"c:\Users\CheEe.Fang\Desktop\csd3401f25_team_alcosoft\Algin\Algin\Assets\DemoAssets\cube.agstaticmesh";
        if (File.Exists(path))
        {
            byte[] bytes = File.ReadAllBytes(path);
            string text = System.Text.Encoding.ASCII.GetString(bytes);
            
            // Look for anything that looks like a material or submesh name
            // Common formats: "Box01_mat0", "lambert1", "Mesh_mat0"
            MatchCollection matches = Regex.Matches(text, @"[a-zA-Z0-9_-]+_mat[0-9]*");
            foreach (Match m in matches)
            {
                Console.WriteLine("FOUND_MAT: " + m.Value);
            }
            
            MatchCollection allStrings = Regex.Matches(text, @"[a-zA-Z0-9_\-]+");
            Console.WriteLine("--- OTHER STRINGS ---");
            int count = 0;
            foreach (Match m in allStrings)
            {
                if (m.Value.Length > 4 && !m.Value.Contains("000")) {
                    Console.WriteLine(m.Value);
                    count++;
                    if (count > 20) break;
                }
            }
        }
    }
}
